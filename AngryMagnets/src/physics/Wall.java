package physics;
import java.awt.*;

/*
 * Walls will form the boundaries of various levels. Some walls can move.
 * --David
 *
 * I haven't tested moving walls yet, but so far this works very well.
 * --Nathaniel Verhaaren, 5-8-2012
 */

public class Wall extends Collideable
{
	//bouncing wall boundaries
	private double leftbound, rightbound, topbound, bottombound;


	public Wall(Vector2D UL, Vector2D LR)
	{
		// The way this is set up is sort of redundant, but it works.
		super('R');
		makeRectangle(UL,LR);
		
		this.setVelocity(Vector2D.ZERO);
	}


	// The way this works is left and right are the x=a forms of the boundaries the Wall cannot cross,
	// and speed is the x-coordinate of the speed.
	// Note that it is up to Environment to enforce these boundaries and give the velocity meaning with each
	// call to step().
	public void bounceX (double left, double right, double speed)
	{
		this.setVelocity(new Vector2D(speed,this.getVelocity().y()));
		leftbound = left;
		rightbound = right;
	}
	
	// Similar to bounceX. Remember that bottom has a higher coordinate than top
	public void bounceY (double top, double bottom, double speed)
	{
		this.setVelocity(new Vector2D(this.getVelocity().x(),speed));
		topbound = top;
		bottombound = bottom;
	}

	public void checkForBounds()
	{
		//Check to see if the rectangle has moved beyond the horizontal boundaries set.
		if ( (this.getULVertex().x() <= leftbound) || (this.getLRVertex().x() >= rightbound) )
			this.setVelocity(new Vector2D(this.getVelocity().x() * -1, this.getVelocity().y()));

		//Check to see if the rectangle has moved beyond the vertical boundaries set.
		if ( (this.getULVertex().y() <= topbound) || (this.getLRVertex().y() >= bottombound) )
			this.setVelocity(new Vector2D(this.getVelocity().x(), this.getVelocity().y() * -1));
	}

	@Override
	public void paint( Graphics g )
	{
		g.setColor(Color.black);
		
		int x = (int)(getULVertex().x());
		int y = (int)(getULVertex().y());
		int width = (int)(getLRVertex().x()) - x;
		int length = (int)(getLRVertex().y()) - y;
		g.fillRect( x, y, width, length );
	}

}
