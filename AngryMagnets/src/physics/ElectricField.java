package physics;

import java.awt.*;

/*
 * ElectricField extends Field and uses the laws of physics to determine forces on charged particles.
 * -- Nathaniel Verhaaren, 4-23-2012
 */

public class ElectricField extends Field 
{
	//the fields we are working with are regular and rectangular, so we only need directions and bounds
	private Vector2D field;
	
	public ElectricField( Vector2D f, Vector2D ulp, Vector2D lrp )
	{
		field = f;
		setULBounds( ulp );
		setLRBounds( lrp );
	}
	
	//It it's out of bounds the force is zero; else F = qE ( Force is charge times electric field ).
	@Override
	public Vector2D getForce(Particle p) 
	{
		if ( inBounds( p.getPosition() ) )
			return field.scale( p.getCharge() );
		else
			return Vector2D.ZERO;
	}
	
	@Override
	public void paint( Graphics page )
	{
		double direction=field.direction();

		double angle=0;
		if(direction>=0 && direction<=Math.PI/2 || direction>=Math.PI && direction<=Math.PI*3/2)
			angle=Math.PI/2-direction;
		else
			angle=Math.abs(Math.PI/2-direction);

	//	System.out.println(angle);
	//UL=upper left, LR=lower right
		double ULx=getULCorner().x();
		double ULy=getULCorner().y();
		double LRx=getLRCorner().x();
		double LRy=getLRCorner().y();
		double width=LRx-ULx;
		double height=LRy-ULy;
		double x1,x2,y1,y2;


		page.setColor(new Color( 0x0, 0x0, 0xFF, 0xAA));
		page.drawRect((int)ULx,(int)ULy,(int) width,(int)height);
		double factor=Math.cos(direction)*50;


//------------------------------------------------------
//below is the code for a completely horizontal field
//------------------------------------------------------
		if(direction==Math.PI|| direction==Math.PI*2 ||direction==0)
		{
			x1=ULx;
			y1=ULy;
			x2=LRx;
			y2=ULy;
			for(int i=0;i<10;y1+=height/10,y2+=height/10, i++)
			{
				page.drawLine((int)(x1),(int)(y1),(int)(x2),(int)(y2));
			}

		}
//------------------------------------------------------
//below is the code for a completely vertical field
//------------------------------------------------------

	else
		if(direction==Math.PI/2|| direction==Math.PI*3/2 )
		{
			x1=ULx;
			y1=ULy;
			x2=ULx;
			y2=LRy;
			for(int i=0;i<10;x1+=width/10,x2+=width/10, i++)
			{
				page.drawLine((int)(x1),(int)(y1),(int)(x2),(int)(y2));
			}

		}


//------------------------------------------------------
//below is the code if in quadrants 1 or 3
//------------------------------------------------------
	else
		if(direction<=Math.PI/2 || direction>=Math.PI && direction <=Math.PI*3/2)
		{

		//sets the initial coordinates and begins to draw the field
			x1=ULx;
			y1=ULx;
			y2=height+ULy;
			x2=(y2-ULy)*Math.tan(angle)+ULx;



			for(x1=ULx;x1<(LRx);x1+=factor)
			{
				if(x2>=(LRx-1))
				{
					x2=width+ULx;
					y2=1/Math.tan(angle)*(x2-x1)+ULy;
	/*				midx=((x2-x1)/2);
					midy=((y2-y1)/2);
					end1x=midx+5;
					end2x=midx;
					end1y=midy;
					end2y=midy+5;
					x[1]=(int)(midx);
					x[2]=(int)(end1x);
					x[3]=(int)(end2x);
					y[1]=(int)(midy);
					y[2]=(int)(end1y);
					y[3]=(int)(end2y); */

					page.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
		//			page.drawPolygon(x,y,3);
				}

				else
				{
					page.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
					y2=height+ULy;
					x2+=factor;
				}

			}

		//resets coordinates and completes the field lines
			x1=ULx;
			x2=ULx+width;
			y2=1/Math.tan(angle)*(x2-ULx)+ULy;

			for(y1=ULy; y1<LRy;y1+=(factor*Math.tan(direction)) )
			{
				if(y2<LRy)
				{
					page.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
					x2=width+ULx;
					y2+=(factor*Math.tan(direction));

				}

				else
				{

					y2=height+ULx;
					x2=Math.tan(angle)*(y2-y1)+ULx;
					page.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
				}

			}
		}
//--------------------------------------
//below is the code if in quadrants 2 or 4
//--------------------------------------

		else
			if(direction<=Math.PI && direction>=Math.PI/2 || direction>=Math.PI*3/2 && direction <=Math.PI*2)
			{

			//sets the initial coordinates and begins to draw the field
				x1=LRx;
				y1=ULy;
				y2=height+ULy;
				x2=LRx-(y2-ULy)*Math.tan(angle);

				for(x1=LRx;x1>(ULx);x1-=(-factor))
				{
					if(x2<=(ULx+2))
					{

						x2=ULx;
						y2=-1/Math.tan(angle)*(x2-x1)+ULy;
						page.drawLine((int)(x1),(int)(y1),(int)(x2),(int)(y2));
					}

					else
					{
						page.drawLine((int)(x1),(int)(y1),(int)(x2),(int)(y2));
						y2=height+ULy;
						x2-=(-factor);

					}

				}

			//resets coordinates and completes the field lines
				x1=LRx;
				y1=ULy;
				y2=height+ULy;
				x2=LRx-(y2-ULy)*Math.tan(angle);

				if(x2<ULx)
					x2=ULx;
				y2=-1/Math.tan(angle)*(x2-x1)+ULy;

				for(y1=ULy; y1<LRy;y1+=(factor*Math.tan(direction)))
				{

					if(y2<(LRy))
					{
						page.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
						x2=ULx;
						y2+=(factor*Math.tan(direction));
					}

					else
					{
						y2=LRy;
						x2=LRx-Math.tan(angle)*(y2-y1);
						page.drawLine((int)x1,(int)y1,(int)x2,(int)y2);
					}

				}
			}
	}
	
	public Vector2D getFieldVector()
	{
		return field;
	}
	
	public void setFieldVector( Vector2D f )
	{
		field = f;
	}
	
}
