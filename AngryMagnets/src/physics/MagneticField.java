package physics;

import java.awt.*;
/*
 * MagneticField extends Field to simulate the effects of a magnetic field on charged particles,
 * which depends on their charge, their velocity, and the direction and magnitude of the field.
 * --Nathaniel Verhaaren, 4-23-2012
 */

public class MagneticField extends Field 
{
	//The strength of the field, which must be perpendicular to the velocity of the particle.
	//Positive is up, negative is down
	private double field;
	
	public MagneticField( double f, Vector2D ulp, Vector2D lrp )
	{
		field = f;
		setULBounds( ulp );
		setLRBounds( lrp );
	}
	
	@Override
	public Vector2D getForce( Particle p )
	{
		// F=q(v x B). The force is orthogonal to the original velocity, and also orthogonal
		// to B, the magnetic field vector, which is perpendicular to the screen, so the force is
		// always in the plane of the screen
		if ( inBounds( p.getPosition() ) )
			return new Vector2D( p.getVelocity().y(), -p.getVelocity().x() ).scale( field*p.getCharge() );
		else
			return Vector2D.ZERO;
	}
	
	@Override
	public void paint( Graphics page )
	{
		double direction=getFieldComponent();

		//UL=upper left, LR=lower right
		double ULx=getULCorner().x();
		double ULy=getULCorner().y();
		double LRx=getLRCorner().x();
		double LRy=getLRCorner().y();

		double width=LRx-ULx;
		double height=LRy-ULy;
		//below creates a box around the field
		page.setColor(new Color(0x0, 0xFF, 0x0, 0xAA));
		page.drawRect((int)ULx,(int)ULy,(int) width,(int)height);


		//draws circles if field is out of page
		if(direction>0)
			for(double x=ULx,y=ULy,i=0; i<20;i++)
			{
				for(int j=0; j<20; j++)
				{
					page.drawOval((int)(x+j*width/20+width/60),(int)y,(int)(width/40),(int)(width/40));
					page.fillOval((int)(x+j*width/20+width/60+width/160), (int)(y+width/160), (int)(width/80), (int)(width/80));
				}
				y+=height/20;
			}

		//draws x's if field is into page
		else
			if(direction<0)
				for(double x=ULx,y=ULy,i=0; i<20;i++)
				{
					for(int j=0; j<20; j++)
					{
						page.drawLine((int)(x+j*width/20+width/60),(int)y,(int)(x+j*width/20+width/60+width/40),(int)(y+width/(40)));
						page.drawLine((int)(x+j*width/20+width/60),(int)(y+width/(40)),(int)(x+j*width/20+width/60+width/40),(int)y);
					}
					y+=height/20;
				}
	}
	
	public double getFieldComponent()
	{
		return field;
	}
	
	public void setFieldComponent( double f )
	{
		field = f;
	}
}
