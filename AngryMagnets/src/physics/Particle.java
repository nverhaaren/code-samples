package physics;

import java.awt.*;
/*
 * Particle contains enough data to simulate a charged point particle.
 * It has a position, velocity, charge, and mass. The mass cannot
 * be less than zero.
 * -- Nathaniel Verhaaren, 4-23-2012
 * 
 * Now some of these attributes are inherited from Collideable
 * -- Nathaniel Verhaaren, 5-7-2012
 */

public class Particle extends Collideable
{
	private double charge;

	public Particle( Vector2D p, Vector2D v, double c, double m )
	{
		// This sort of thing is generally considered bad coding style, but whatever.
		super( 'P' );
		
		// by default, isRooted is true, which makes these immune to collisions, which is bad.
		this.setRooted( false );

		position = p;
		setVelocity(v);
		charge = c;

		// Collideable may allow negative masses, but a particle's must be >= 0
		if ( m >= 0 )
			setMass(m);
		else
			setMass(-m);
	}

	public Particle()
	{
		this( Vector2D.ZERO, Vector2D.ZERO, 0, 1 );
	}

	public Vector2D getPosition()
	{
		return position;
	}

	public double getCharge()
	{
		return charge;
	}


	public void setPosition( Vector2D p )
	{
		position = p;
	}


	public void setCharge( double c )
	{
		charge = c;
	}

	public void setMass( double m )
	{
		if ( m >= 0 )
			super.setMass(m);
		else
			super.setMass(-m);
	}
	
	public double getKEnergy()
	{
		// KE = m*v*v/2
		return 0.5 * getMass() * getVelocity().dot(getVelocity());
	}
	
	public Vector2D getMomentum()
	{
		// p = m*v
		return getVelocity().scale(getMass());
	}
	
	@Override
	public void paint( Graphics g )
	{
		if ( charge > 0.0 )
			g.setColor( Color.RED );
		else if ( charge == 0.0 )
			g.setColor( Color.GREEN );
		else
			g.setColor( Color.BLUE );
		
		g.fillOval( (int)getPosition().x() - 2, (int)getPosition().y() - 2, 4, 4 );
	}

	@Override
	public boolean equals( Object o )
	{
		if ( !( o instanceof Particle ) )
			return false;
		else
		{
			Particle p = (Particle)o;
			if ( this.getMass() == p.getMass()  &&  this.charge == p.getCharge() )
				return true;
			else
				return false;
		}
	}
	
	@Override
	public int hashCode()
	{
		return (int)getMass() + (int)charge;
	}

	@Override
	public String toString()
	{
		return position.toString() + "\t\t" + getVelocity().norm();
	}

}
