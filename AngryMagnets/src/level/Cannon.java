package level;
import physics.*;
import java.awt.*;

public class Cannon 
{	
	public final static double radius = 50.0;
	public final static double maxSpeed = 100.0;
	
	private Environment sim;
	
	private Vector2D position;
	
	private double minAngle;
	private double maxAngle;
	private double currentAngle;
	private double currentSpeed;
	
	private double particleMass = 1.0;
	private double particleCharge = 1.0;
	
	public Cannon( Environment s, Vector2D loc, double min, double max )
	{
		sim = s;
		
		position = loc;
		
		minAngle = min;
		maxAngle = max;
		
		currentAngle = 0.0;
		currentSpeed = 0.0;
	}
	
	public Environment getEnv()
	{
		return sim;
	}
	
	public Vector2D getPosition()
	{
		return position;
	}
	
	public double getPartMass()
	{
		return particleMass;
	}
	public void setPartMass( double mass )
	{
		particleMass = mass;
	}
	
	public double getPartCharge()
	{
		return particleCharge;
	}
	public void setPartCharge( double charge )
	{
		particleCharge = charge;
	}
	
	public double getMinAngle()
	{
		return minAngle;
	}
	public double getMaxAngle()
	{
		return maxAngle;
	}
	public double getCurrentAngle()
	{
		return currentAngle;
	}
	public double getCurrentSpeed()
	{
		return currentSpeed;
	}
	
	public void decSpeed()
	{
		currentSpeed -= 0.5;
		if ( currentSpeed < 0.0 )
			currentSpeed = 0.0;
	}
	public void incSpeed()
	{
		currentSpeed += 0.5;
		if ( currentSpeed > maxSpeed )
			currentSpeed = maxSpeed;
	}
	public void decAngle()
	{
		currentAngle -= 1.5 * (Math.PI / 180.0);
		if ( currentAngle < minAngle )
			currentAngle = minAngle;
	}
	public void incAngle()
	{
		currentAngle += 1.5 * (Math.PI / 180.0);
		if ( currentAngle > maxAngle )
			currentAngle = maxAngle;
	}
	
	public void fire()
	{
		Vector2D partPosition = new Vector2D( Math.cos(currentAngle), Math.sin(currentAngle) ).scale(radius).plus(position);
		Vector2D partVelocity = new Vector2D( Math.cos(currentAngle), Math.sin(currentAngle) ).scale( currentSpeed );
		
		Particle p = new Particle( partPosition, partVelocity, particleCharge, particleMass );
		
		sim.addParticle(p);
		
		//currentSpeed -= 0.2 * maxSpeed;
		//if ( currentSpeed < 0.0 )
		//	currentSpeed = 0.0;
	}
	
	public void paint( Graphics g )
	{
		double ULx,ULy,LLx,LLy,URx,URy,LRx,LRy;
		double MURx, MURy, MLRx, MLRy;
		
		double x = position.x();
		double y = position.y();

		//NOTE: This should work if the angle is greater than zero
		//the midpoint of the back of the cannon is the location of the cannon
		// NOTE: This works and is awesome. Thanks, Ben.
		// Also, I have made the cannon itself the meter.

		//these are the endpoints of the back of the cannon
		ULx=x-5*Math.sin(currentAngle);
		ULy=y+5*Math.cos(currentAngle);
		LLx=x+5*Math.sin(currentAngle);
		LLy=y-5*Math.cos(currentAngle);

		//these are the endpoints of the front of the cannon
		URx=ULx+radius*Math.cos(currentAngle);
		URy=ULy+radius*Math.sin(currentAngle);
		LRx=LLx+radius*Math.cos(currentAngle);
		LRy=LLy+radius*Math.sin(currentAngle);
		
		//these are the endpoints of the meter
		double mradius = radius * currentSpeed / maxSpeed;
		MURx = ULx + mradius * Math.cos(currentAngle);
		MURy = ULy + mradius * Math.sin(currentAngle);
		MLRx = LLx + mradius * Math.cos(currentAngle);
		MLRy = LLy + mradius * Math.sin(currentAngle);
		

		int[] ax=new int[4];

		ax[0]=(int)ULx;
		ax[1]=(int)LLx;
		ax[2]=(int)LRx;
		ax[3]=(int)URx;

		int[] ay=new int[4];

		ay[0]=(int)ULy;
		ay[1]=(int)LLy;
		ay[2]=(int)LRy;
		ay[3]=(int)URy;
		
		int ax2[] = { (int)ULx, (int)LLx, (int)MLRx, (int)MURx };
		int ay2[] = { (int)ULy, (int)LLy, (int)MLRy, (int)MURy };

		g.setColor(Color.RED);
		g.fillPolygon(ax,ay,4);
		
		g.setColor(new Color( 0x0, 0xAA, 0x0));
		g.fillPolygon(ax2, ay2, 4);

	}
}
