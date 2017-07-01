package physics;

import java.awt.Graphics;

import level.Antimatter;

/*
 * Collideable contains the framework of objects that will be capable of colliding with each other.
 * A series of booleans will determine exactly what the properties of the object is.
 * This will be used as an abstract class that things like walls and obstacles will be made from,
 * as well as the particles we are shooting themselves. Anything that can be collided into will be
 * an extension of this class.
 * -- David Traviolia, 4-27-2012
 * 
 * David and I both worked on combining Collideable with the rest of the classes in physics.
 * -- Nathaniel Verhaaren, 5-7-2012
 * 
 * I worked to improve functionality in collisions with walls.
 * -- Nathaniel Verhaaren, 5-8-2012
 */

public abstract class Collideable
{
	private boolean isApoint;
	private boolean isArectangle;

	protected Vector2D position;

	protected Vector2D upperLeft;
	protected Vector2D bottomRight;

	private Vector2D velocity;
	private double mass;

	// a rooted object is not affected by collisions, but it may affect them.
	private boolean isRooted;

	public Collideable(char choice)
	{
		switch (choice)
		{
			case('R'):
				makeRectangle();
				break;
			case('P'):
				makePoint();
				break;
			default:
				//This should never happen!!
				try
				{
					throw new Exception("You cannot call the Collideable constructor without supplying 'R', or 'P'!!");
				}
				catch (Exception e)
				{
				}
		}
	}

	//Sets Collideable values to represent a circle - at some point we may want this.
	/*
	private void makeCircle()
	{
		isApoint = false;
		isArectangle = false;
		isRooted = true;
	}
	public void makeCircle(Vector2D location)
	{
		isApoint = false;
		isArectangle = false;
		isRooted = true;
		position = location;
	}
	*/

	//Sets Collideable values to represent a rectangle
	private void makeRectangle()
	{
		isApoint = false;
		isArectangle = true;
		isRooted = true;
	}
	public void makeRectangle(Vector2D ULcorner, Vector2D LRcorner)
	{
		isApoint = false;
		isArectangle = true;
		isRooted = true;
		upperLeft = ULcorner;
		bottomRight = LRcorner;
	}


	//Sets Collideable values to represent a point
	private void makePoint()
	{
		isApoint = true;
		isArectangle = false;
		isRooted = false;
	}
	public void makePoint(Vector2D location)
	{
		isApoint = true;
		isArectangle = false;
		isRooted = false;
		position = location;
	}

	public void setPosition(Vector2D p)
	{
		position = p;
	}
	public Vector2D getPosition()
	{
		return position;
	}

	public void setULVertex( Vector2D UL )
	{
		upperLeft = UL;
	}
	public Vector2D getULVertex()
	{
		return upperLeft;
	}

	public void setLRVertex( Vector2D LR )
	{
		bottomRight = LR;
	}
	public Vector2D getLRVertex()
	{
		return bottomRight;
	}


	public void setVelocity(Vector2D vel)
	{
		velocity = vel;
	}
	public Vector2D getVelocity()
	{
		return velocity;
	}


	public double getMass()
	{
		return mass;
	}
	public void setMass(double input)
	{
		mass = input;
	}


	public boolean checkRooted()
	{
		return isRooted;
	}
	public void setRooted(boolean choice)
	{
		isRooted = choice;
	}


	// Note that these fields cannot be changed after the solid is created
	// Okay, actually they can, but they shouldn't be.
	public boolean checkRectangle()
	{
		return isArectangle;
	}
	public boolean checkPoint()
	{
		return isApoint;
	}

	public void collision( Collideable second )
	{
		
		if (this.checkRooted() && second.checkRooted())
			return;
		
		// This code allows different walls to have certain effects
		// Or it would have, except Antimatter is the only one and this caused problems later.
		// If we ever have lots of types of walls, we should change this entire method, really.
		if ( second instanceof Antimatter )
		{
			second.collision( this );
			return;
		}
		
		if (second.checkRooted())
		{
			Vector2D collisionPoint = this.position;
			int Xpos = (int) collisionPoint.x();
			int Ypos = (int) collisionPoint.y();

			//the end Velocity of the incident particle
			Vector2D reflectedRay;

			//
			if (second.checkRectangle())
			{
				int top, left, bottom, right;
				top = (int) second.upperLeft.y();
				left = (int) second.upperLeft.x();
				bottom = (int) second.bottomRight.y();
				right = (int) second.bottomRight.x();

				// if these start to malfunction I know how to write more precise code
				// but I hope these shorter and cheaper methods will work.
				if (Xpos > left && Xpos < right && Ypos <= bottom && Ypos >= top)
					reflectedRay = new Vector2D(this.velocity.x(), this.velocity.y()*-1).plus(second.velocity);
				else
					reflectedRay = new Vector2D(this.velocity.x()*-1, this.velocity.y()).plus(second.velocity);

				this.velocity = reflectedRay;

			}

		}
		else if (this.checkRooted())
		{
			second.collision(this);
		}
		else
		{
			// As of now, I have no idea how well this works
			// Sometime we may wish to check...
			double x1, y1, x2, y2;
			x2 = second.getVelocity().x() * second.getMass();
			y2 = second.getVelocity().y() * second.getMass();
			x1 = this.getVelocity().x() * this.getMass();
			y1 = this.getVelocity().y() * this.getMass();

			second.velocity = new Vector2D(x1/second.getMass(), y1/second.getMass());
			this.velocity = new Vector2D(x2/this.getMass(), y2/this.getMass());

		}
	}
	
	public abstract void paint( Graphics g );

}
