package physics;

import java.awt.Graphics;

/* 	Field is a class used to represent fields in the physics sense.
	Both ElectricField and MagneticField will extend from it.
	If a particle is given to it, getForce will return the Force
	that the field exerts on the particle.
	--Nathaniel Verhaaren, 4-23-2012
*/

public abstract class Field 
{
	//these specify the rectangle
	private Vector2D ulPosition;
	private Vector2D lrPosition;
	
	//returns true if the given vector corresponds to a position inside its rectangle
	//of influence. Although it is rectangular, it cannot collide so is not Collideable.
	//I don't know if David modeled his rectangles off this, but I did not model these of his,
	//as this was written first.
	
	//Actually, I wrote the similar helper method to check for collisions in the Environment class
	//without even remembering this method. It has virtually the same code, though.
	public boolean inBounds( Vector2D position )
	{
		if ( position.x() > ulPosition.x()  &&  position.x() < lrPosition.x()  &&  
				position.y() > ulPosition.y()  &&  position.y() < lrPosition.y() )
			return true;
		else
			return false;
	}
	
	public Vector2D getULCorner()
	{
		return ulPosition;
	}
	
	public Vector2D getLRCorner()
	{
		return lrPosition;
	}
	
	public void setULBounds( Vector2D ulp )
	{
		ulPosition = ulp;
	}
	
	public void setLRBounds( Vector2D lrp )
	{
		lrPosition = lrp;
	}
	
	public abstract Vector2D getForce( Particle p );
	
	public abstract void paint( Graphics g );
}
