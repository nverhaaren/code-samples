package level;
import physics.*;
import java.awt.*;

// This is a class used to represent the position and radius of an explosion

public class Explosion 
{
	private Vector2D position;
	private double radius;
	
	private int timesPainted;
	
	public Explosion( Vector2D p, double r )
	{
		position = p;
		radius = r;
		timesPainted = 0;
	}
	
	public Vector2D getPosition()
	{
		return position;
	}
	
	public double getRadius()
	{
		return radius;
	}
	
	public int getTimesPainted()
	{
		return timesPainted;
	}
	
	public double getArea()
	{
		return Math.PI * radius * radius;
	}
	
	public void paint( Graphics g )
	{
		timesPainted++;
		
		int x=(int)getPosition().x();
		int y=(int)getPosition().y();
		int r1 = (int)( radius * ((double)timesPainted / 100.0) );
		if ( timesPainted <= 100 )	
		{
			if( timesPainted<30)
				g.setColor(Color.RED);
			else if(timesPainted<70)
					g.setColor(Color.ORANGE);
			else
				g.setColor(Color.YELLOW);
			g.fillOval(x - r1,y - r1,2*r1,2*r1);
		}
	}
}
