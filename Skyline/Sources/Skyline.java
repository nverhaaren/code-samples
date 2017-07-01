import javax.swing.*;
import java.util.*;


public class Skyline extends JFrame
{
	// Eclipse wanted this
	private static final long serialVersionUID = 1L;

	// Stores what time is considered dawn
	private Date zeroTime;
	// Where we draw everything
	private TerminaCanvas canvas;
	// Number of milliseconds since zeroTime
	private long time;
	// System time at the last update of the canvas
	private long timeOfLastUpdate;

	public static void main( String[] args )
	{
		// creates the window
		Skyline frame = new Skyline( "Majora" );
		frame.setDefaultCloseOperation( EXIT_ON_CLOSE );

		// put the canvas in the window
		frame.addComponentsToFrame();

		// make the frame respect the canvas' size
		frame.pack();
		frame.setResizable( false );
		frame.setVisible( true );
		// tell the canvas its being displayed so that it can create the off-screen buffer
		frame.canvas.setIsDisplayed( true );

		int iHour, ai[];

		while ( true )
		{
			// set the time
			frame.time = (int) ( System.currentTimeMillis() - frame.zeroTime.getTime() );

			// synchronize the canvas' time with our time
			frame.canvas.setTime( frame.time );

			// calculate what hour of the day the Skyline will reflect
			iHour = (int) ( ( ( frame.time / 1000 ) / 60 ) + 6 ) % 24;

			// paint every quarter (or twentieth) of a second
			if ( System.currentTimeMillis() - frame.timeOfLastUpdate > 50 )
			{
				// now is when the last update happened for next time
				frame.timeOfLastUpdate = System.currentTimeMillis();

				// determine whether to turn windows on or off, depending on whether it's day or night
				boolean lit = !( ( ( ( iHour - 6 ) % 24 ) + 24 ) % 24 < 12 );

				//System.out.print( iHour + "\t" );
				// use iHour again to for determining window behavior
				iHour = ( ( ( ( iHour - 6 ) % 12 ) + 12 ) % 12 ) + 6;
				//System.out.println( iHour + "\t" );

				// turn some windows on or off - lots on during night, off during day, none at sunset/rise
				for ( int i = 0; i < 5 * ( 6 - Math.abs( 12 - iHour ) ); i++ )
				{
					ai = TerminaCanvas.getRandomWindowCoordinate();
					frame.canvas.setWindow(ai[0], ai[1], ai[2], lit);
				}

				// toggle some windows during dusk
				for ( int i = 0; i < 5 * Math.abs( 12 - iHour ); i++ )
				{
					ai = TerminaCanvas.getRandomWindowCoordinate();
					frame.canvas.setWindow( ai[0], ai[1], ai[2], !frame.canvas.getWindow( ai[0], ai[1], ai[2] ) );
				}

				// repaint the canvas
				frame.canvas.repaint();
			}
		}

	}

	public Skyline( String name )
	{
		// call the superclass' constructor with the window name
		super( name );

		// create a new canvas to draw on
		canvas = new TerminaCanvas();

		// set the zeroTime as now - a Date object holds the time it was initialized
		zeroTime = new Date();

		time = 0;
		timeOfLastUpdate = zeroTime.getTime();
	}

	private void addComponentsToFrame()
	{
		this.add( canvas );
	}
}


