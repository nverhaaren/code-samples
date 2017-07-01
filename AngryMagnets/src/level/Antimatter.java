package level;

import physics.*;
import java.util.*;
import java.awt.*;

/*
 * Antimatter will cause particles to explode, which is primarily how
 * the user will be attempting to kill enemies.
 * --David
 * 
 * It creates a collection of explosions caused by particles colliding with it,
 * which Level will check to see which Enemies and Particles need to be removed.
 */

public class Antimatter extends Wall
{
	private ArrayList<Explosion> explosions;
	
	public Antimatter(Vector2D UL, Vector2D LR)
	{
		super(UL,LR);
		
		explosions = new ArrayList<Explosion>();
	}

	public void collision( Collideable second )
	{
		if (second instanceof Particle)
		{
			Particle p = (Particle)second;
			explosions.add(new Explosion( p.getPosition(), Math.sqrt( p.getKEnergy() ) / 2 ));
			p.setVelocity(Vector2D.ZERO);
		}
		else
		{
			super.collision(second);
		}
	}
	
	public Explosion takeTopExplosion()
	{
		if ( explosions.size() == 0 )
			return null;
		else
			return explosions.remove(explosions.size() - 1);
	}
	
	public boolean unresolvedExplosions()
	{
		return explosions.size() != 0;
	}
	
	@Override
	public void paint( Graphics g )
	{
		g.setColor(Color.magenta);
		
		int x = (int)(getULVertex().x());
		int y = (int)(getULVertex().y());
		int width = (int)(getLRVertex().x()) - x;
		int length = (int)(getLRVertex().y()) - y;
		g.fillRect( x, y, width, length );
	}
}

