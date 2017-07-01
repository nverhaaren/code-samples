import java.awt.*;
import java.util.*;



public class TerminaCanvas extends Canvas
{
	// Automatically generated ID:
	private static final long serialVersionUID = 1L;
	
	// random generator for the stars
	private static Random generator;
	
	// the dimension this wants to be
	private Dimension preferredDimension;
	
	// the time the canvas will display, in milliseconds since first dawn
	private long time;
	
	// various colors I use
	private final Color groundColor = new Color( 0, 0x99, 0);
	private final Color buildingColor = new Color( 0x40, 0x40, 0x40 );
	private final Color daySkyColor = new Color( 36, 83, 242 );
	private final Color nightSkyColor = new Color( 0x06, 0, 0x40 );
	private final Color litWindowColor = new Color( 0xFF, 0xFF, 0xBB );
	private final Color normalClockDark = new Color( 24, 131, 107 );
	private final Color normalClock = new Color( 31, 171, 139 );
	private final Color darkWindowColor = Color.black;
	private final Color sunColor = Color.red;
	
	// the buildings will be 100 pixels wide
	private final int BUILDING_WIDTH = 100;
	
	// holds an array of coordinates of stars
	private Dimension stars[];
	
	// window constants I use (generated based on dimensions every call to paint() )
	private int GROUND;
	private int CENTER;
	private int HEIGHT;
	private int WIDTH;
	private int BTW_WIDTH;
	private int BUILDING_HEIGHT;
	
	// an array saying whether windows will be on or off
	private boolean windows[][][];
	
	// for use in double-buffering
	private Image buffer;
	private Graphics bufferDrawer;
	
	// a boolean holding whether the canvas is being displayed
	private boolean isDisplayed;
	
	// Originally the canvas user could set the size, but I decided against it.
	// To avoid recoding I just marked this constructor private.
	private TerminaCanvas( Dimension preferredDimension )
	{
		this.preferredDimension = preferredDimension;
		this.setFocusable( true );
		
		// four buildings
		windows = new boolean[4][][];
		
		// each building has its own dimensions
		windows[0] = new boolean[5][20];
		windows[1] = new boolean[5][16];
		windows[2] = new boolean[5][23];
		windows[3] = new boolean[5][20];
		
		// set all the windows to unlit.
		for( int i = 0; i < windows.length; i++ )
			for( int j = 0; j < windows[i].length; j++ )
				for( int k = 0; k < windows[i][j].length; k++ )
					windows[i][j][k] = false;
		
		generator = new Random();
		
		// create and initialize 500 stars to random positions in the sky
		stars = new Dimension[500];
		for( int i = 0; i < stars.length; i++ )
			stars[i] = new Dimension( generator.nextInt(701), generator.nextInt(401) );
	}
	
	public TerminaCanvas()
	{
		// calls the other constructor with a size of 700 x 500
		this( new Dimension( 700, 500) );
	}
	
	public Dimension getPreferredSize()
	{
		// this way the JFrame can respect its size wishes
		return preferredDimension;
	}
	
	public void paint( Graphics g )
	{
		// prevents null pointer exceptions with the double buffer
		if ( !isDisplayed )
			return;
		
		// establishes what the size is
		Dimension size = getSize();
		
		// sets the window constants based on the size
		GROUND = (int) ( size.getHeight() * 0.8 );
		CENTER = (int) ( size.getWidth() / 2 );
		HEIGHT = (int) ( size.getHeight() );
		WIDTH = (int) ( size.getWidth() );
		BTW_WIDTH = ( WIDTH - 5 * BUILDING_WIDTH ) / 6;
		
		// paint the ground offscreen
		paintBelowBackground( bufferDrawer );
		
		// paint the sky offscreen
		paintAboveBackground( bufferDrawer );
		
		// paint the clock offscreen
		paintClock( bufferDrawer );
		
		// paint the buildings offscreen
		paintBuildings( bufferDrawer );
		
		// paint the offscreen image onscreen
		g.drawImage( buffer, 0, 0, new Color( 0, 0, 0, 0 ), this );
		
	}
	
	public void update( Graphics g )
	{
		// prevents the background being cleared every time I call repaint()
		paint( g );
	}
	
	private void paintAboveBackground( Graphics page )
	{	
		// gets the sky color for the hour of the day
		page.setColor( getSkyColor( (int) ( ( ( time / 1000 ) / 60 ) + 6 ) % 24 ) );
		
		page.fillRect( 0, 0, WIDTH, 4 * HEIGHT / 5 );
		
		// paint stars
		
		page.setColor( Color.white );
		
		// only at night
		if ( ( time / 1000 / 60 ) % 24 >= 12 )
		{
			for( Dimension d : stars )
			{
				page.fillOval( d.width, d.height, 1, 2 );
			}
		}
	}
	
	private void paintBelowBackground( Graphics page )
	{
		// paint ground
		page.setColor( groundColor );
		page.fillRect( 0, GROUND, WIDTH, HEIGHT - GROUND );
	}
	
	private void paintBuildings( Graphics page )
	{
		page.setColor( buildingColor );
		BTW_WIDTH = ( WIDTH - 5 * BUILDING_WIDTH ) / 6;
		
		// use a for loop to draw the building
		// i is the number of the building, x the farthest left coordinate
		for( int i = 0, x = BTW_WIDTH; i < 4; i++, x+= ( BTW_WIDTH + BUILDING_WIDTH ) )
		{
			// leave a gap for the clock tower (which I didn't end up drawing)
			if ( i == 2 )
				x += ( BTW_WIDTH + BUILDING_WIDTH );
			
			switch ( i )
			{
				case 0:
					BUILDING_HEIGHT = ( HEIGHT * 3 ) / 5;
					break;
				case 1:
					BUILDING_HEIGHT = ( HEIGHT * 12 ) / 25;
					break;
				case 2:
					BUILDING_HEIGHT = ( HEIGHT * 7 ) / 10;
					break;
				case 3:
					BUILDING_HEIGHT = ( HEIGHT * 3) / 5;
					break;
				default:
					System.out.println( "Error in paintBuildings()!" );
					BUILDING_HEIGHT = 0;
					break;
			}
			
			page.fillRect( x, 4 * HEIGHT / 5 - BUILDING_HEIGHT, BUILDING_WIDTH, BUILDING_HEIGHT );
		}
		paintWindows( page );
		BUILDING_HEIGHT = 0;
	}
	
	private void paintWindows( Graphics page )
	{
		int NUM_STORIES[] = { 20, 16, 23, 20 };
		
		
		// runs similarly to paintBuildings()
		for( int i = 0, x = BTW_WIDTH; i < 4; i++, x += ( BTW_WIDTH + BUILDING_WIDTH ) )
		{
			if ( i == 2)
				x += ( BTW_WIDTH + BUILDING_WIDTH );
			
			switch ( i )
			{
				case 0:
					BUILDING_HEIGHT = ( HEIGHT * 3 ) / 5;
					break;
				case 1:
					BUILDING_HEIGHT = ( HEIGHT * 12 ) / 25;
					break;
				case 2:
					BUILDING_HEIGHT = ( HEIGHT * 7 ) / 10;
					break;
				case 3:
					BUILDING_HEIGHT = ( HEIGHT * 3) / 5;
					break;
				default:
					System.out.println( "Error in paintBuildings()!" );
					BUILDING_HEIGHT = 0;
					break;
			}
			
			// generate the dimensions of each window
			double WINDOW_HEIGHT = (double)BUILDING_HEIGHT / ( 2 * NUM_STORIES[i] + 1);
			double WINDOW_WIDTH = (double)BUILDING_WIDTH / ( 2 * 5 + 1 );	// 5 windows per story
			
			// draw the windows
			for( int j = 0; j < NUM_STORIES[i]; j++ )
			{
				for( int k = 0; k < 5; k++ )
				{
					// chose which color, depending on if dark or lit
					if ( windows[i][k][j] )		// windows specifies by building, x, y
						page.setColor( litWindowColor );
					else
						page.setColor( darkWindowColor );
					
					page.fillRect( x + (int) ( ( 2 * k + 1 ) * WINDOW_WIDTH ) , 4 * HEIGHT / 5 - BUILDING_HEIGHT +
							(int) ( ( 2 * j + 1 ) * WINDOW_HEIGHT ), (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT );
				}
			}
		}
	}
	
	public void paintClock( Graphics g )
	{
		// useful coordinates
		final int LEFT = 3 * BTW_WIDTH + 2 * BUILDING_WIDTH;
		final int RIGHT = LEFT + BUILDING_WIDTH;
		final int TOP = GROUND;
		final int BOTTOM = HEIGHT;
		
		// draw the background diamonds
		int xCoords[] = { LEFT, ( LEFT + RIGHT ) / 2, RIGHT, ( LEFT + RIGHT ) / 2 }, 
				yCoords[] = { ( TOP + BOTTOM ) / 2, TOP, ( TOP + BOTTOM ) / 2, BOTTOM };
		
		g.setColor( normalClockDark );
		g.fillPolygon( xCoords, yCoords, 4 );
		
		xCoords[0] += 10;
		xCoords[2] -= 10;
		
		yCoords[1] += 10;
		yCoords[3] -= 10;
		
		g.setColor( normalClock );
		g.fillPolygon( xCoords, yCoords, 4 );
		
		// draw the frame
		g.setColor( Color.yellow );
		g.drawArc( LEFT - BUILDING_WIDTH / 2, TOP, 2 * BUILDING_WIDTH, 2 * BUILDING_WIDTH, 0, 180 );
		
		for( int i = 0; i <= 180; i += 15 )
		{
			g.drawLine( CENTER, HEIGHT, CENTER + (int)( BUILDING_WIDTH * Math.cos( (double)i / 180 * Math.PI ) ),
					HEIGHT - (int)( BUILDING_WIDTH * Math.sin( (double)i / 180 * Math.PI ) ) );
		}
		
		// calculate the minute and hour to display
		
		int minute = (int) ( ( time / 1000 ) % 60 );
		int hour = (int) ( ( time / 1000 ) / 60 ) % 24;
		
		// calculated the angles (in radians ) the orbs are at
		double mcd = Math.PI / 2 - 3.0 * minute / 180 * Math.PI;
		double hcd = Math.PI - 15.0 * ( hour + (double)minute / 60 ) / 180 * Math.PI;
		
		// draw small minutes sun
		
		// draws two - one in window, one out of bounds
		// (This is done this way because this clock is a clone of that found in LOZ Majora's Mask
		g.fillOval( CENTER + (int) ( (double)BUILDING_WIDTH / 2 * Math.cos( mcd ) ) - 10,
				HEIGHT - (int) ( (double)BUILDING_WIDTH / 2 * Math.sin( mcd ) ) - 10, 20, 20);
		g.fillOval( CENTER - (int) ( (double)BUILDING_WIDTH / 2 * Math.cos( mcd ) ) - 10,
				HEIGHT + (int) ( (double)BUILDING_WIDTH / 2 * Math.sin( mcd ) ) - 10, 20, 20);
		
		// draw large, red hours sun ( visible during day )
		
		g.setColor( sunColor );
		g.fillOval( CENTER + (int) ( (double)BUILDING_WIDTH * Math.cos( hcd ) ) - 20,
				HEIGHT - (int) ( (double)BUILDING_WIDTH * Math.sin( hcd ) ) - 20, 40, 40 );
		
		// draw face
		g.setColor( Color.black );
		g.fillOval( CENTER + (int) ( ( (double)BUILDING_WIDTH + 5 ) * Math.cos( hcd + 3 * Math.PI / 180 ) ) - 2,
				HEIGHT - (int) ( ( (double)BUILDING_WIDTH + 5 ) * Math.sin( hcd + 3 * Math.PI / 180 ) ) - 2, 4, 4 );
		g.fillOval( CENTER + (int) ( ( (double)BUILDING_WIDTH + 5 ) * Math.cos( hcd - 3 * Math.PI / 180 ) ) - 2,
				HEIGHT - (int) ( ( (double)BUILDING_WIDTH + 5 ) * Math.sin( hcd - 3 * Math.PI / 180 ) ) - 2, 4, 4 );
		
		g.drawArc( CENTER + (int) ( ( (double)BUILDING_WIDTH - 2 ) * Math.cos( hcd ) ) - 7,
				HEIGHT - (int) ( ( (double)BUILDING_WIDTH - 2  ) * Math.sin( hcd ) ) - 7, 14, 14,
				( (int) ( hcd / Math.PI * 180 ) - 220 ) % 360, 80 );
		
		// draw yellow, night hours crescent moon ( visible during night )
		
		g.setColor( Color.yellow );
		g.fillOval( CENTER - (int) ( (double)BUILDING_WIDTH * Math.cos( hcd ) ) - 20,
				HEIGHT + (int) ( (double)BUILDING_WIDTH * Math.sin( hcd ) ) - 20, 40, 40 );
		
		// draw the 'shadow'
		g.setColor( nightSkyColor.darker().darker() );
		g.fillOval( CENTER - (int) ( ( (double)BUILDING_WIDTH + 4 ) * Math.cos( hcd ) ) - 16,
				HEIGHT + (int) ( ( (double)BUILDING_WIDTH + 4 ) * Math.sin( hcd ) ) - 16, 32, 32 );
		
		// draw face
		g.setColor( Color.black );
		g.fillOval( CENTER - (int) ( ( (double)BUILDING_WIDTH - 14 ) * Math.cos( hcd + 4 * Math.PI / 180 ) ) - 2,
				HEIGHT + (int) ( ( (double)BUILDING_WIDTH - 14 ) * Math.sin( hcd + 4 * Math.PI / 180 ) ) - 2, 4, 4 );
		g.drawLine( CENTER - (int) ( ( (double)BUILDING_WIDTH - 12 ) * Math.cos( hcd ) ),
				HEIGHT + (int) ( ( (double)BUILDING_WIDTH - 12 ) * Math.sin( hcd ) ),
				CENTER - (int) ( ( (double)BUILDING_WIDTH - 20 ) * Math.cos( hcd ) ),
				HEIGHT + (int) ( ( (double)BUILDING_WIDTH - 20 ) * Math.sin( hcd ) ) );
	}
	
	public long getTime()
	{
		return time;
	}
	
	public void setTime( long iTime )
	{
		this.time = iTime;
	}
	
	public boolean getWindow( int i, int j, int k )
	{
		return windows[i][j][k];
	}
	
	public void setWindow( int i, int j, int k, boolean isLit )
	{
		windows[i][j][k] = isLit;
	}
	
	public static int[] getRandomWindowCoordinate()
	{
		int i = generator.nextInt( 5 * ( 20 + 16 + 23 + 20) );
		int ret[] = { 0, i % 5, 0 };
		
		if ( i < 5 * 20 )
		{
			ret[0] = 0;
			ret[2] = i / 5;
		} else if ( i < 5 * 20 + 5 * 16 ) {
			ret[0] = 1;
			ret[2] = ( i - 5 * 20 ) / 5;
		} else if ( i < 5 * 20 + 5 * 16 + 5 * 23 ) {
			ret[0] = 2;
			ret[2] = ( i - 5 * 20 - 5 * 16 ) / 5;
		} else {
			ret[0] = 3;
			ret[2] = ( i - 5 * 20 - 5 * 16 - 5 * 23 ) / 5;
		}
		
		return ret;
	}

	public void setIsDisplayed( boolean flag )
	{
		// this creates and destroys offscreen buffer resources as appropriate,
		// as well as changing the flag
		isDisplayed = flag;
		if ( flag )
		{
			if ( buffer == null )
			{
				// create an offscreen image of the same size as the window
				buffer = createImage( (int) getSize().getWidth(), (int) getSize().getHeight() );
				
				// create a Graphics object that accesses it
				bufferDrawer = buffer.getGraphics();
				
				// set the background as transparent
				bufferDrawer.setColor( new Color( 0, 0, 0, 0 ) );
				bufferDrawer.fillRect( 0, 0, WIDTH, HEIGHT );
			}
		} else {
			if ( buffer != null )
			{
				// get rid of resources, delete objects
				bufferDrawer.dispose();
				buffer.flush();
				buffer = null;
				bufferDrawer = null;
			}
		}
	}
	
	public boolean getIsDisplayed()
	{
		return isDisplayed;
	}

	private Color getSkyColor( int hour )
	{
		
		hour -= 6;
		hour %= 24;
		boolean isNight = ( hour >= 12 );
		hour %= 12;
		hour += 6;
		hour = 6 - Math.abs( 12 - hour );
		if ( !isNight )
			hour += 6;
		else
			hour = 6 - hour;
		// now hour contains how much day (out of 12) the color should have
		
		// and the method returns on a sliding scale from midnight to noon
		return new Color( ( ( 12 - hour ) * nightSkyColor.getRed() +
				hour * daySkyColor.getRed() ) / 12,
				( ( 12 - hour ) * nightSkyColor.getGreen() +
				hour * daySkyColor.getGreen() ) / 12,
				( ( 12 - hour ) * nightSkyColor.getBlue() +
				hour * daySkyColor.getBlue() ) / 12 );
	}

}
