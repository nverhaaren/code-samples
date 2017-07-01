package physics;
import java.util.ArrayList;

/*
 * This is a first draft of the simulator for the physics engine. It can simulate the effects of electric
 * and magnetic fields on charged particles using numerical approximations of the solutions of differential
 * equations used in physics. The tolerance is the step between each moment of time; the lower it is, the more
 * accurate the simulation will be.
 * -- Nathaniel Verhaaren, 4-24-2012
 * 
 * Collisions are now dealt with using David's Collideable class
 * -- Nathaniel Verhaaren, 5-7-2012
 * 
 * We are not going to check the interactions of every particle with every other
 * because the complexity would increase quadratically. It may be worth it to assign each Particle a permanent
 * force vector so that we do not redo equal and opposite force calculations.
 * -- Nathaniel Verhaaren, 5-8-2012
 * 
 * And now we're ignoring particle-particle forces, because we can if we fiddle with universal constants,
 * and hey, it's our universe.
 * -- Nathaniel Verhaaren, 5-12-2012
 */

public class Environment
{
	// all elements of particles should also be listed under solids
	private ArrayList<Field> fields;
	private ArrayList<Particle> particles;
	private ArrayList<Collideable> solids;

	//The length of time between each simulation
	private double tolerance;
	
	// The tolerance times the number of steps passed so far.
	private double time;
	
	// in the future, we'll use this to determine the edges
	private Vector2D bounds;

	public Environment( double tol, Vector2D LRVertex )
	{
		time = 0.0;
		tolerance = tol;
		bounds = LRVertex;
		fields = new ArrayList<Field>();
		particles = new ArrayList<Particle>();
		solids = new ArrayList<Collideable>();
	}

	public void addField( Field f )
	{
		fields.add( f );
	}

	public void addParticle( Particle p )
	{
		// any particle is also a solid
		particles.add( p );
		solids.add(p);
	}

	public void addSolid( Collideable c )
	{
		// those solids that are particles should be recognized as such
		if ( c instanceof Particle )
			this.addParticle( (Particle)c );
		solids.add( c );
	}
	
	public void reset()
	{
		fields.clear();
		particles.clear();
		solids.clear();
		
		time = 0.0;
	}

	public ArrayList<Field> getFields()
	{
		return fields;
	}

	public ArrayList<Particle> getParticles()
	{
		return particles;
	}

	public ArrayList<Collideable> getSolids()
	{
		return solids;
	}

	public double getTime()
	{
		return time;
	}

	public double getTolerance()
	{
		return tolerance;
	}
	
	public Vector2D getBounds()
	{
		return bounds;
	}

	//calling this method advances the simulation by one tolerance
	public void step()
	{
		// moves all the particles according to their forces
		stepParticles();
		
		// move all the solids according to their velocities and other properties
		stepSolids();

		// advance the simulation's internal clock
		time += tolerance;

		// Then check for collisions:
		
		// This checks for collisions using a private helper method
		// If there should be a collision, David's method is called
		for( int i = 0; i < solids.size() - 1; i++ )
		{
			for( int j = i + 1; j < solids.size(); j++ )
			{
				if ( checkCollision( solids.get( i ), solids.get( j ) ) )
					solids.get( i ).collision( solids.get( j ) );
			}
		}

	}
	
	private void stepParticles()
	{
		// since right now we are only planning to have one particle at a time, this ignores
		// the effects of other particles. If we have multiple particles at a time, I may have
		// to look up specifics regarding magnetism.
		
		// Now we are going to pretend that we have set e0 and m0 to values such that 
		// particle-particle forces are negligible, which will speed up simulations
		for( Particle p : particles )
		{
			// This represents the total force
			Vector2D force = new Vector2D();

			// Which is the sum of all the forces combined.
			// Currently only including fields
			for( Field f : fields )
			{
				force = force.plus( f.getForce(p) );
			}

			// F = m*a
			Vector2D a = force.scale( 1.0 / p.getMass() );

			// Numerically approximate the next position and velocity. The lower tolerance, the more accurate
			p.setPosition( p.getPosition().plus( a.scale( 0.5 * tolerance * tolerance ) ).
					plus( p.getVelocity().scale( tolerance ) ) );
			p.setVelocity( p.getVelocity().plus( a.scale( tolerance ) ) );
		}
		
		for( int i = 0; i < particles.size(); i++ )
		{
			if ( !checkPointInRectangle(particles.get(i).getPosition(), Vector2D.ZERO, bounds) )
			{
				for(int j = 0; j < solids.size(); j++)
				{
					if ( solids.get(j) == particles.get(i))
					{
						solids.remove(j);
						break;
					}
				}
				particles.remove(i);
				i--;
			}
		}
	}
	
	private void stepSolids()
	{
		// This does not check for collisions, but since solids have velocity, position and mass
		// they also can move
		for( Collideable c : solids )
		{
			// Particles are already taken care of
			if ( !(c instanceof Particle) )
			{
				// points are simple
				if ( c.checkPoint() )
					c.setPosition(c.getPosition().plus(c.getVelocity().scale(tolerance)));
				else if ( c.checkRectangle() )
				{
					// rectangles are more complicated
					c.setULVertex(c.getULVertex().plus(c.getVelocity().scale(tolerance)));
					c.setLRVertex(c.getLRVertex().plus(c.getVelocity().scale(tolerance)));
					
					// and some walls automatically bounce off of invisible boundaries
					if ( c instanceof Wall )
						((Wall)c).checkForBounds();
				}
			}
		}
	}

	//Private helper method to see if a and b overlap, using checkPointInRectangle()
	private static boolean checkCollision( Collideable a, Collideable b )
	{
		if ( a.checkRectangle() )
		{
			if ( b.checkPoint() )
			{
				//pt-rect collision; only check if the point is anywhere in the rectangle
				if ( checkPointInRectangle( b.getPosition(), a.getULVertex(), a.getLRVertex() ) )
					return true;
				else
					return false;
			} else {
				//rect-rect collision; realistically we only need to check to see if the
				//opposing vertices are inside the other rectangle
				if ( checkPointInRectangle( b.getULVertex(), a.getULVertex(), a.getLRVertex() ) ||
					 checkPointInRectangle( b.getLRVertex(), a.getULVertex(), a.getLRVertex() ) ||
					 checkPointInRectangle( a.getULVertex(), b.getULVertex(), b.getLRVertex() ) ||
					 checkPointInRectangle( a.getLRVertex(), b.getULVertex(), b.getLRVertex() ) )

					return true;
				else
					return false;
			}
		} else {
			if ( b.checkPoint() )
			{
				//pt-pt collision; they must occupy the same pixel space.
				if ( (int)(b.getPosition().x()) == (int)(a.getPosition().x()) &&
					 (int)(b.getPosition().y()) == (int)(a.getPosition().y()) )

					return true;
				else
					return false;
			} else {
				// pt-rect collision again
				if ( checkPointInRectangle( a.getPosition(), b.getULVertex(), b.getLRVertex() ) )
					return true;
				else
					return false;
			}
		}
	}

	// private helper method that checks if the point is in the rectangle determined by the two vectors
	public static boolean checkPointInRectangle( Vector2D pt, Vector2D ul, Vector2D lr )
	{
		if ((int)(pt.x()) >= (int)(ul.x())  &&
			(int)(pt.x()) <= (int)(lr.x())  &&
			(int)(pt.y()) >= (int)(ul.y())  &&
			(int)(pt.y()) <= (int)(lr.y()))

			return true;
		else
			return false;
	}

}
