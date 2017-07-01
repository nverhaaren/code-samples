import java.awt.*;
import java.util.*;

import physics.*;
import level.*;

// now this class needs only some work.

public class SimulatorCanvas extends Canvas 
{
	/**
	 * Automatically generated ID
	 */
	private static final long serialVersionUID = 1L;
	
	private Dimension preferredDimension;
	
	private Environment sim;
	private ArrayList<Level> levels;
	private Level currentLevel;
	private int levelIndex;
	private boolean started;
	private boolean finished;
	private boolean gameOver;
	
	private ArrayList<Explosion> explosions = new ArrayList<Explosion>();
	
	private Image buffer;
	private Graphics bufferDrawer;
	
	private boolean isDisplayed;
	
	public int shotCount = 0;
	
	//private long time;
	
	
	
	public SimulatorCanvas( Environment s, ArrayList<Level> l )
	{
		sim = s;
		levels = l;
		levelIndex = 0;
		currentLevel = levels.get( levelIndex );
		started = false;
		finished = false;
		
		preferredDimension = new Dimension( 700, 500 );
		
		// This is necessary for keyboard input to work!
		this.setFocusable( false );
	}
	
	public Dimension getPreferredSize()
	{
		return preferredDimension;
	}
	
	public Level getCurrentLevel()
	{
		return currentLevel;
	}
	
	public boolean started()
	{
		return started;
	}
	
	public boolean finished()
	{
		return finished;
	}
	
	public void nextLevel()
	{
		//System.out.println( "Moving to the next level!" );
		levelIndex++;
		if ( levelIndex == levels.size() )
		{
			finished = true;
		} else if ( levelIndex > levels.size() ) {
			System.exit(0);
		} else {
			explosions.clear();
			currentLevel = levels.get(levelIndex);
			currentLevel.start();
		}
	}
	
	public void start()
	{
		started = true;
		levels.get(levelIndex).start();
	}
	
	public void paint( Graphics g )
	{
		if ( !isDisplayed )
			return;
		
		if ( !started )
		{
			// code for painting title screen
			bufferDrawer.setColor( new Color( 0x00, 0x00, 0x00) );
			bufferDrawer.fillRect(0, 0, this.getWidth(), this.getHeight());
			bufferDrawer.setColor(Color.GREEN);
			bufferDrawer.drawString("Angry Magnets",300,200);
			bufferDrawer.drawString("Press 'Enter' to begin", 280, 220);
		}

		if ( started && !finished && !gameOver )
		{
		
			if ( currentLevel.gameOver() )
			{
				gameOver = true;
				return;
			}
			
			if ( currentLevel.finished() )
			{
				// code for painting level complete screen
				bufferDrawer.setColor( new Color( 0x0, 0x0, 0x0) );
				bufferDrawer.fillRect(0, 0, this.getWidth(), this.getHeight());
				bufferDrawer.setColor(Color.GREEN);
				bufferDrawer.drawString("Level Complete!",300,200);
				bufferDrawer.drawString("Press 'Enter' to continue", 280, 220);
			}
			else
			{
				// for some reason 10's are necessary, at least on my home laptop.
				// not at school, though, for which this is built. Curse you, Microsoft and/or
				// Sun Microsystems
				bufferDrawer.clearRect(0, 0, this.getWidth(), this.getHeight());
				
				//clear the screen
				bufferDrawer.setColor( new Color( 0xFF, 0xFF, 0xFF) );
				bufferDrawer.fillRect(0, 0, this.getWidth(), this.getHeight());
				
				// add in any new explosions that need to be drawn.
				if ( currentLevel.unpaintedExplosions() )
					explosions.addAll( currentLevel.takeNewExplosions() );
				
				
				
				
				try {
					
				paintFields( bufferDrawer );
				paintCannon( bufferDrawer );
				paintCollideables( bufferDrawer );
				paintExplosions( bufferDrawer );
				} catch ( Exception ConcurrentModificationException )
				{
					//System.out.println( "yada yada...");
				}
				
				// draw the enemy 'target'
				bufferDrawer.setColor(Color.orange);
				bufferDrawer.drawOval((int)currentLevel.getEnemyGoal().x() - 10, (int) currentLevel.getEnemyGoal().y() - 10, 20, 20);
				bufferDrawer.drawOval((int)currentLevel.getEnemyGoal().x() - 5, (int) currentLevel.getEnemyGoal().y() - 5, 10, 10);
				bufferDrawer.drawOval((int)currentLevel.getEnemyGoal().x() - 1, (int) currentLevel.getEnemyGoal().y() - 1, 2, 2);
			}
		}
		
		if ( finished )
		{
			// code for painting credits and ending screen
			bufferDrawer.setColor( new Color( 0x0, 0x0, 0x0) );
			bufferDrawer.fillRect(0, 0, 700, 500);
			bufferDrawer.setColor(Color.GREEN);
			bufferDrawer.drawString("Congratulations! You Win!",300,200);
			bufferDrawer.drawString("Created by:",300,250);
			bufferDrawer.drawString("Nathaniel Verhaaren",300,275);
			bufferDrawer.drawString("Ben Paren",300,300);
			bufferDrawer.drawString("David Traviolia",300,325);
			bufferDrawer.drawString("You took " + shotCount + " shots", 300, 400);
		}
		
		if ( gameOver )
		{
			// code for painting a 'you lose' screen
			bufferDrawer.setColor( new Color( 0x0, 0x0, 0x0) );
			bufferDrawer.fillRect(0, 0, 700, 500);
			bufferDrawer.setColor(Color.GREEN);
			bufferDrawer.drawString("You Lose",300,200);
		}
			
		// Without this line of code, we see nothing.
		g.drawImage( buffer, 0, 0, new Color( 0, 0, 0, 0), this );
	}
	
	private void paintCollideables( Graphics g )
	{
		for( Collideable c : sim.getSolids() )
		{
			c.paint(g);
		}
	}
	
	private void paintFields( Graphics g )
	{
		for ( Field f : sim.getFields() )
		{
			f.paint(g);
		}
	}
	
	private void paintCannon( Graphics g )
	{
		currentLevel.getCannon().paint(g);
	}
	
	private void paintExplosions( Graphics g )
	{
		for ( int i = 0; i < explosions.size(); i++ )
		{
			explosions.get(i).paint(g);
			if ( explosions.get(i).getTimesPainted() > 100 )
			{
				explosions.remove(i);
				i--;
			}
		}
	}


	public void update( Graphics g )
	{
		// For some reason it is important to override this method this way.
		paint( g );
	}
	
	public void setIsDisplayed( boolean flag )
	{
		isDisplayed = flag;
		
		// These resources (for the double buffer) cannot be allocated until there is a window visible.
		// We destroy them when the window is no longer visible so as not to take up valuable memory
		if( flag )
		{
			if ( buffer == null )
			{
				//Debugging tool:
				//System.out.println( "width: " + getSize().getWidth() + "length: " + getSize().getHeight() );
				
				// Create the buffer
				buffer = createImage( (int)getSize().getWidth(), (int)getSize().getHeight() );
				
				// Create some Graphics to access the buffer
				bufferDrawer = buffer.getGraphics();
				
				// Draw nothing (probably not necessary)
				bufferDrawer.setColor( new Color( 0, 0, 0, 0 ) );
				bufferDrawer.fillRect( 0, 0, (int)(getSize().getWidth()), (int)(getSize().getHeight()) );
			}
			
		} else {
			if ( buffer != null )
			{
				// dispose of the resources and get rid of the references
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
	
}
