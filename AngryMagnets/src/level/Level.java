package level;

import physics.*;
import java.util.*;

/*
 * The Level class creates a level, which will provide instances of itself as the various levels.
 * It also holds and creates one Cannon object
 * 
 * Enemies are the big things that need to be worked on. David, make sure they follow precisely the correct path. Enemies
 * should emerge at the beginning once every enemyPeriod seconds. numberEnemies is the fixed limit to all the enemies in the
 * level, numberKilled is the number killed so far, the ArrayList enemies contains a list of those currently on-screen.
 * If they reach their goal, gameOver should be set to true. If they are all eliminated, finished should be set to true.
 * -- Nathaniel Verhaaren, May 14, 2012
 */

public class Level
{
	private Environment backboard;

	private int numberEnemies;
	//private int numberKilled;
	private int enemyPeriod; //make sure this is in units of time, not # of tolerance.
	private double lastTime; // used in stepEnemies & constructor.
	
	private ArrayList<Enemy> enemies = new ArrayList<Enemy>();
	private ArrayList<Wall> walls = new ArrayList<Wall>();
	private ArrayList<Antimatter> booms = new ArrayList<Antimatter>();
	private ArrayList<Field> fields = new ArrayList<Field>();
	private ArrayList<Explosion> unpaintedExplosions = new ArrayList<Explosion>();
	
	private ArrayList<Vector2D> targetPath = new ArrayList<Vector2D>();
	private ArrayList<Double> speeds = new ArrayList<Double>();
	
	private boolean started = false;
	private boolean finished = false;
	private boolean gameOver = false;

	private Cannon shooter;

	public static Vector2D makeVelocity(Vector2D start, Vector2D end, double speed)
	{
		return end.plus(start.scale(-1.0)).unit().scale(speed);
	}
	
	public static boolean closeEnough(Vector2D point1, Vector2D point2)
	{
		if(Math.abs(point1.x()-point2.x())<2)
			if(Math.abs(point1.y()-point2.y())<2)
				return true;
		return false;
	}
	
	public static boolean inRange( Vector2D a, Vector2D b, double range )
	{
		return a.plus(b.scale(-1.0)).norm() <= range;
	}
	
	public Level ( Environment homeEnv, Vector2D locCannon, Vector2D spawn, double maxAngCannon, double minAngCannon,
				   int numEnemies, int enPeriod )
	{
		backboard = homeEnv;
		targetPath.add(spawn);
		shooter = new Cannon( homeEnv, locCannon, minAngCannon, maxAngCannon );
		numberEnemies = numEnemies;
		enemyPeriod = enPeriod;
		lastTime = backboard.getTime();
	}
	
	public void addWall( Wall w )
	{
		if ( !started )
		{
			walls.add(w);
			if ( w instanceof Antimatter )
				booms.add((Antimatter)w);
			backboard.addSolid(w);
		}
	}
	
	public void addField( Field f )
	{
		if ( !started )
		{
			fields.add(f);
			backboard.addField(f);
		}
	}
	
	public void addPathPoint(Vector2D position, double speed)
	{
		if ( !started )
		{
			int index = targetPath.size() - 1;
			targetPath.add(position);
			speeds.add(Level.makeVelocity(targetPath.get(index), position, speed).norm());
		}
	}
	
	public void start()
	{
		started = true;
		backboard.reset();
		for ( Field f : fields )
			backboard.addField(f);
		
		for ( Wall w : walls )
		{
			backboard.addSolid(w);
		}
	}

	
	public void step()
	{
		if ( started && !finished && !gameOver )
		{
			backboard.step();
			
			// this for loop deals with explosions
			for ( Antimatter a : booms )
			{
				while ( a.unresolvedExplosions() )
				{
					Explosion e = a.takeTopExplosion();
					for( int i = 0; i < enemies.size(); i++ )
					{
						if ( inRange( e.getPosition(), enemies.get(i).getPosition(), e.getRadius() ) )
						{
							// enemies are also in the solids ArrayList
							for( int j = 0; j < backboard.getSolids().size(); j++ )
							{
								if ( backboard.getSolids().get(j) == enemies.get(i) )
								{
									backboard.getSolids().remove(j);
									break;
								}
							}
							enemies.remove(i);
							i--;
						}
					}
					
					for ( int i = 0; i < backboard.getParticles().size(); i++ )
					{
						if ( closeEnough(backboard.getParticles().get(i).getPosition(), e.getPosition()) )
						{
							// Don't forget particles are in the solids ArrayList too.
							for( int j = 0; j < backboard.getSolids().size(); j++ )
							{
								if ( backboard.getParticles().get(i) == backboard.getSolids().get(j) )
								{
									backboard.getSolids().remove(j);
									break;
								}
							}
							backboard.getParticles().remove(i);
							break;
						}
					}
					
					unpaintedExplosions.add(e);
				}
			}
			
			stepEnemies();
		}
	}


	private void stepEnemies()
	{
		// this method will make sure all enemies are heading towards the right target.
		// and will add new enemies as necessary.(second if statement)
		for( int index = 0; index < enemies.size(); index++)
		{
			int current = enemies.get(index).pathSegment;
			if (Level.closeEnough(enemies.get(index).getPosition(), targetPath.get(current+1)))
			{
				//checks to see if this is the game-ending button.
				if (Level.closeEnough(targetPath.get(current+1),  targetPath.get(targetPath.size()-1)))
				{
					gameOver = true;
					//System.out.println( "Game ended" );
				}
				else
				{
					//speeds ArrayList no longer is designed as an ArrayList<double>, which is impossible.
					// it is instead an ArrayList<Double>, which is possible
					enemies.get(index).setVelocity(Level.makeVelocity(enemies.get(index).getPosition(),
																	targetPath.get(current+2),
																	speeds.get(current+1)));
					enemies.get(index).pathSegment++;

					//above line calculates a velocity from the given Enemy to the next point in the
					//path and sets it to that Enemy. It only triggers if Enemy is close enough to its
					//target point, as seen in the closeEnough() method.
				}
			}

		}
		//adds enemies at the appropriate time.
		if ((backboard.getTime() >= enemyPeriod + lastTime) && (numberEnemies > 0))
		{
			lastTime = backboard.getTime();
			Enemy e = new Enemy();
			e.setPosition(targetPath.get(0));
			e.setVelocity(Level.makeVelocity(e.getPosition(),
											targetPath.get(1),
											speeds.get(0)));
			enemies.add(e);
			backboard.addSolid(e);
			numberEnemies--;
		}

		//check here to see if level ends by either all enemies destroyed or one reaching the end.
		// involving setting gameOver and finished as appropriate.
		//-- from david: the enemies arraylist holds enemies CURRENTLY in the level.
		//-------numberEnemies holds the number of Enemies YET to be introduced.
		if ((enemies.size() == 0) && (numberEnemies <= 0))
		{
			finished=true;
			
			walls.clear();
			booms.clear();
			fields.clear();
			unpaintedExplosions.clear();
			targetPath.clear();
			speeds.clear();
			//System.out.println("level ended");
		}
	}
	
	
	public boolean finished()
	{
		return finished;
	}
	
	public boolean started()
	{
		return started;
	}
	
	
	public boolean gameOver()
	{
		return gameOver;
	}

	public Cannon getCannon()
	{
		return shooter;
	}
	
	public Vector2D getEnemyGoal()
	{
		if ( !finished )
			return this.targetPath.get(targetPath.size()-1);
		else
			return Vector2D.ZERO;
	}
	
	public ArrayList<Explosion> takeNewExplosions()
	{
		// This reassigns a reference to a new variable while returning a reference to
		// the old variable
		
		ArrayList<Explosion> old = unpaintedExplosions;
		unpaintedExplosions = new ArrayList<Explosion>();
		
		return old;
	}
	
	public boolean unpaintedExplosions()
	{
		return unpaintedExplosions.size() != 0;
	}

}
