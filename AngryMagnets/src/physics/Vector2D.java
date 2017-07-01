package physics;

/*
 * This immutable class represents a two-dimensional vector of doubles.
 * It also contains common vector operations.
 * -- Nathaniel Verhaaren, 4-23-2012
 * 
 * Now has equals() and toString()
 * -- Nathaniel Verhaaren, 4-24-2012
 */

public final class Vector2D
{
	private final double x;
	private final double y;
	
	public static final Vector2D ZERO = new Vector2D();
	// NaV means Not a Vector, like NaN means Not a Number
	public static final Vector2D NaV = new Vector2D( Double.NaN, Double.NaN );
	
	public Vector2D( double x, double y )
	{
		this.x = x;
		this.y = y;
	}
	
	public Vector2D()
	{
		this( 0.0, 0.0 );
	}
	
	public double x()
	{
		return x;
	}
	
	public double y()
	{
		return y;
	}
	
	// returns its counterclockwise angle with horizontal (east) in radians.
	public double direction()
	{
		double theta = Math.atan2( y, x );
		if ( theta < 0 )
			theta += 2 * Math.PI;
		return theta;
	}
	
	// returns k * this
	public Vector2D scale( double k )
	{
		return new Vector2D( k * x, k * y );
	}
	
	//returns the length of the vector
	public double norm()
	{
		return Math.sqrt( x * x + y * y );
	}
	
	//returns this vector normalized
	public Vector2D unit()
	{
		if ( !(this.equals(ZERO)) )
			return this.scale( 1.0 / this.norm() );
		else
			return NaV;
	}
	
	//returns this dot b
	public double dot( Vector2D b )
	{
		return this.x() * b.x() + this.y() + b.y();
	}
	
	//returns the k-component of this cross b
	public double cross( Vector2D b )
	{
		return this.x() * b.y() - this.y() * b.x();
	}
	
	//returns the sum of two vectors
	public Vector2D plus( Vector2D b )
	{
		return new Vector2D( this.x() + b.x(), this.y() + b.y() );
	}
	
	//returns this projected onto b
	public Vector2D proj( Vector2D b )
	{
		if ( b.equals(ZERO) )
			return NaV;
		else
			return b.unit().scale( this.dot( b ) / b.norm() );
	}
	
	@Override
	public boolean equals( Object o )
	{
		if ( ! ( o instanceof Vector2D ) )
			return false;
		else
		{
			Vector2D v = (Vector2D)o;
			if ( x == v.x()  &&  y == v.y() )
				return true;
			else
				return false;
		}
	}
	
	//Java trivia of the day: it is a Bad Thing to override equals without also overriding hashCode
	
	@Override
	public int hashCode()
	{
		return (int)x + (int)y;
	}
	
	@Override
	public String toString()
	{
		return "<" + x + ", " + y + ">";
	}
}
