package level;
import physics.*;
import java.awt.*;
//import java.util.*;
/*
*The Enemy class creates an enemy, which will be treated as a Rooted point
*
*/

public class Enemy extends Collideable
{
	public int pathSegment;

	public Enemy()
	{
		super('P');
		super.setRooted(true);
		pathSegment=0;
	}
	
	@Override
	public void paint( Graphics g )
	{
		double x = position.x();
		double y = position.y();
		
		g.setColor( new Color( 0xFF, 0x0, 0x0, 0xAA) );
		g.fillRect((int)x - 8, (int)y - 8, 16, 16);
		
		g.setColor(Color.BLACK);
		g.drawRect((int)x - 8,(int)y - 8,16,16);
		g.fillOval((int)x - 2, (int)y - 2, 4, 4);
	}
}
