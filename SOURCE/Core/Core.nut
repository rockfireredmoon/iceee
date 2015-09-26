
/*
   The interface specification for the entire player library. All scriptable
   elements in the player will be of one of the following types. Implementation
   of these classes is actually in C++ inside the player itself, but the
   description, interface specification, and all documentation are in this
   file.
   <p>
   The syntax used here is not actually valid Squirrel. It borrows the type
   checking enhancments used in Flash ActionScript 2.0 (which is actually
   ecmaScript based) to convey type information. This means that for every
   parameter, variable, or function declaration, its type will be suffixed
   in the form var:Type. So, for example:
   <code>function mult(x:Integer):Integer {}</code>
   declares a function called "mult" that takes a single integer parameter
   and returns an integer. If a type is optional or can be one of multiple
   types, it may be omitted.
   <p>
   The documentation itself is embedded inside comments near each relevant
   code snippet and conforms to the Javadoc specification (or at least a
   subset of it) to allow easy parsing and generating of human-readable
   documenation later.
   <p>
   For classes that have predefined events (events the system calls), they
   will include a member variable called "events" that will define
   all of the events and default closure implementations for each (that do
   nothing). To hook into an event, you should provide a table/object that
   implements a closure with the same parameter types (if any) and has
   the same name as that in the <code>events</code> table. Then,
   you can add it to an instance using addListener() and it will be called
   when the corresponding event occurs.
*/


/**
	A standard 3-element floating point vector.
*/
class Vector3
{
	/**
		Create a new vector.
		<p>
		Can take the following forms:
		<ul>
		<li>No arguments. Defaults to 0,0,0.</li>
		<li>1 string of 3 floats: "x y z"</li>
		<li>3 floats: x, y, z</li>
		</ul>
	*/
	constructor(...);
	
	/**
		Calculate the length of the vector. Length is defined to be:
		<code>sqrt(x * x + y * y + z * z)</code>. This is a relatively
		slow operation, so if you're sure you don't need the exact length,
		consider using {@link #lengthSquared()}.
		
		@return The length of the vector.
	*/
	</
		SuppressWarnings = "unchecked" 
		Position = {x = "123", y="123" }
	/>
	function length():Float;
	
	/**
		Calculate the squared-length of the vector. This can be used in
		imprecise calculations as a speedy alternative to actual length.
		
		@return The squared magnitude of the length.
	*/
	function lengthSquared():Float;
	
	/**
		Normalise the vector (adjust it so that its length is 1.0). If
		the vector has a length of 0, nothing is done.
		
		@return The previous length of the vector.
	*/
	function normalize():Float;
	
	/**
		Calculate the dot product of this vector with another one.
	*/
	function dot(v:Vector3):Float;
	
	/**
		Create a new vector that is the cross product of this vector
		and another one. The returned vector is not normalized (for
		efficiency's sake). If you require this, do so explicitly.
	*/
	function cross(v:Vector3):Vector3;
	
	/**
		Create a new vector that is the midpoint between this vector
		and the given vector.
	*/
	function midPoint(v:Vector3):Vector3;
	
	/**
		Create a new vector that is the summation of this and the
		given vector.
	*/	
	function _add(v:Vector3):Vector3;
	
	/**
		Create a new vector that is the negative summation (subtraction)
		of this vector and another.
	*/
	function _sub(v:Vector3):Vector3;
	
	/**
		Return the multiplication of this vector with a float value.
	*/
	function _mul(val:Float):Vector3;
	
	/**
		Return the division result of this vector with a scalar value.
	*/
	function _div(val:Float):Vector3;
	
	/**
		Test if the vector is equal with another. Equality is defined as
		piece-wise equality between all components (i.e. a.x==b.x && a.y==b.y && a.z==b.z)
		
		@return True if the vectors are equal.
	*/
	function equals(q:Quaternion):Boolean;
	
	/**
		Invert the sign of each component in this vector.
		It returns itself in order to facilitate operation
		"chaining".
	*/
	function negate():Vector3;
	
	x = 0;
	y = 0;
	z = 0;
}

/**
	A quaternion is used to hold all orientation and rotation values.
*/
class Quaternion
{
	w = 1.0;
	x = 0;
	y = 0;
	z = 0;

	/**
		Create a new quaternion.
		<p>
		Can take the following forms:
		<ul>
		<li>No arguments. Defaults to w=1,x=0,y=0,z=0.</li>
		<li>1 string of 4 floats: "w x y z"</li>
		<li>1 float (angle in radians), 1 Vector3 (axis of rotation)</li>
		<li>4 floats (w, x, y, z)</li>
		</ul>
	*/
	constructor(...):Quaternion;

	/**
		Get the current AngleAxis angle in radians. This is the
		angle component of an AngleAxis pair that can be used
		to represent this quaternion.
	*/
	function getAngle():Float;
	
	/**
		Get the current AngleAxis axis of rotation. This is the
		"axis of rotation" component of an AngleAxis pair that can
		be used to represent this quaternion.
	*/
	function getAxis():Vector3;
	
	/**
		Calculate the local X axis.
		
		@return A vector representing the local coordinate system's X axis vector.
	*/
	function xAxis():Vector3;
	
	/**
		Calculate the local Y axis.
		
		@return A vector representing the local coordinate system's Y axis vector.
	*/
	function yAxis():Vector3;
	
	/**
		Calculate the local Z axis.
		
		@return A vector representing the local coordinate system's Z axis vector.
	*/
	function zAxis():Vector3;
	
	/**
		Calculate the local roll element of this quaternion.
	*/
	function getRoll():Float;
	
	/**
		Calculate the local pitch element of this equation.
	*/
	function getPitch():Float;
	
	/**
		Calculate the local yaw element of this equation.
	*/
	function getYaw():Float;
	
	/**
		Create a new quaternion that is the summation of this and another.
	*/
	function _add(q:Quaternion):Quaternion;
	
	/**
		Create a new quaternion that is the negative summation (subtraction) of
		this and another.
	*/
	function _sub(q:Quaternion):Quaternion;

	/**
		Test if the quaternion is equal with another. Equality is defined as
		piece-wise equality between all components (i.e. a.w==b.w, a.x==b.x, etc)
		
		@return True if the quaternions are equal.
	*/
	function equals(q:Quaternion):Boolean;
	
	/**
		Perform a quaternion multiplication. <i>Quaternion multiplication
		is not commutative.</i> I.e. <code>p * q</code> does not always equal
		<code>q * p</code>.
	*/
	function _mul(q:Quaternion):Quaternion;
	
	/**
		Calculate the dot product between this quaternion and another.
	*/
	function dot(q:Quaternion):Float;
	
	/**
		The mathematical 'norm' of the quaternion. This is defined to be
		x*x+y*y+z*z+w*w, or the squared-distance.
		
		@return The Norm of the quaternion.
	*/
	function norm():Float;
	
	/**
		Normalise the quaternion, forcing its length to be 1.0.
		
		@return The previous length.
	*/
	function normalize():Float;
	
	/**
		Create a new quaternion that is the inverse of this quaternion.
		A quaternion must have a non-zero length in order have an inverse.
	*/
	function inverse():Quaternion;
	
	/**
		Create a new quaternion that is the spherical linear interpolation
		(SLERP) of this quaternion being interpolated 'toward' the
		<code>dst</code> quaternion.
		
		@param t
				The interpolation value, must be between 0 and 1 inclusive.
		@param dst
				The 'goal' quaternion. A <code>t</code> value of 1 will result
				in exactly this quaternion.
				
		@return A new quaternion representing the interpolation between the two.
	*/
	function slerp(t:Float, dst:Quaternion):Quaternion;
}

/**
	A 3D box aligned with the X, Y, and Z axes.
*/
class AxisAlignedBox
{
	/**
		Create a new axis aligned box. This can be done in several different
		ways:
		<ol>
		<li>none: Default: empty box.</li>
		<li>2 Vector3 values: minimum and maximum extents</li>
		<li>6 float values: min x, y, z, and max x, y, z</li>
		</ol>
	*/
	constructor(...);
	
	/**
		Get the minimum extent of this box.
	*/
	function getMinimum():Vector3;
	
	/**
		Set the minimum extent of this box.
	*/
	function setMinimum(value:Vector3);
	
	/**
		Get the maximum extent of this box.
	*/
	function getMaximum():Vector3;
	
	/**
		Set the maximum extent of this box.
	*/
	function setMaximum(value:Vector3);
	
	/**
		Set both the minimum and maximum extents of this box.
	*/
	function setExtents(min:Vector3, max:Vector3);
	
	/**
		Incorporate the given box or point into the extents of this one,
		expanding as necessary.
		<p>
		Parameters can be:
		<ul>
		<li>AxisAlignedBox - merge with the entire box</li>
		<li>Vector3 - merge the given point</li>
		<li>3 floats - merge the given point</li>
		</ul>
	*/
//	function merge(...); (Not implemented yet)
	
	/**
		Reset this box to the empty set.
	*/
	function setNull();
	
	/**
		Does the box even have an extent?
	*/
	function isNull();
	
	/**
		Test whether this box fully contains another box or a specific point.
		<p>
		Parameter can be:
		<ul>
		<li>AxisAlignedBox - test another box</li>
		<li>Vector3 - test a point</li>
		<li>x,y,z - 3 floats defining a point to test</li>
		</ul>
	*/
	function contains(...):Boolean;
	
	/**
		Test whether this box intersects any part of various geometric
		primitives (points, boxes, etc).
		<p>
		The parameters can be:
		<ul>
		<li>AxisAlignedBox - another potentially overlapping box</li>
		<li>Vector3 - whether the box includes the given point (same as contains)</li>
		<li>Vector3, radius:Float - a sphere with the given center point and radius</li>
		</ul>		
	*/
	function intersects(...):Boolean;
	
	/**
		Return the intersection of this box and another.
	*/
	function intersection(box:AxisAlignedBox):AxisAlignedBox;

	/**
		Get the center of this box.
	*/
	function getCenter():Vector3;	
}

/**
	A 3D sphere, centered at a given point, having a radius R.
*/
class Sphere
{
	/**
		Create a new sphere. Valid constructor variants are:
		<ul>
		<li>Sphere() - A default unit sphere at the origin.</li>
		<li>Sphere(center:Vector3, radius:Float) - An initial center and radius.</li>
		<li>Sphere(x:Float, y:Float, z:Float, radius:Float) - An initial center and radius.</li> 
		</ul>
		
	*/
	constructor(...);

	/**
		Get the sphere's center point.
	*/
	function getCenter():Vector3;		
	
	/**
		Set the sphere's center point.
	*/
	function setCenter(center:Vector3);
	
	/**
		Get the sphere's radius.
	*/
	function getRadius():Float;
	
	/**
		Get the sphere's radius.
	*/
	function setRadius(radius:Float);
	
	/**
		Perform some mathematical intersection tests against this sphere.
		Valid arguments are:
		<ul>
		<li>Sphere - Intersect another sphere</li>
		<li>AxisAlignedBox - Intersect a box</li>
		<li>Vector3 - See if the point is within this sphere.</li>
		<li>x:Float,y:Float,z:Float - See if the point is within this sphere.</li>
		</ul>
	*/
	function intersects(...):Boolean;
}


/**
	A color value, represented by its Red, Blue, Green, and Alpha components.
*/
class Color
{
	/**
		Create a new color value. For all values, 
		<p>
		All numeric values are expected to be floating point values.
		<p>
		Constructor variants:
		<ul>
		<li>none: Default color (white), 1.0,1.0,1.0,1.0.</li>
		<li>1 string value, hex representation, (one of): RRGGBB, RGB, RGBA, RRGGBBAA</li>
		<li>3 numeric values: r, g, b.</li>
		<li>4 numeric values: r, g, b, a.</li>
		</ul>
	*/
	constructor(...);

	/**
		Clamp a single color value into a valid 0.0 - 1.0 range. This will also
		convert a 0 - 255 integer value into the normalized float
		representation (0.0 - 1.0).
		
		@return A float value betwen 0 and 1, inclusive.
	*/
	static function clamp(x):Float;

	/**
		Clamp all color values to the 0.0 - 1.0 range.
	*/
	function saturate();
	
	/**
		Get the packed-integer representation of this color value. Each
		color is converted to the 0-255 range and bit shifted into the
		corresponding bits of a single integer.
		<p>
		Note: HDR (high dynamic range) lighting values (i.e. those outside
		the 0-1 range) are lost in this conversion if present.
		
		@return An integer representing this color in RGBA format.
	*/
	function getAsRGBA():Integer;
	
	/**
		Set the colour values of this color from a packed-integer in the
		RGBA format.
		
		@param rgba
				The red, green, blue, and alpha components in a
				packed integer representation.
	*/
	function setAsRGBA(rgba:Integer);
	
	/**
		Set the color values from a Hue/Saturation/Brightness (HSB) triplet.
		
		@param hue
				The hue component (0 - 1.0)
		@param saturation
				The saturation component (0 - 1.0)
		@param brightness
				The brightness component (0 - 1.0)
	*/
	function setHSB(hue:Float, saturation:Float, brightness:Float);
	
	/**
		Get the hexadecimal representation of this string. This is either
		a 6 or 8 digit hexadecimal number in either RRGGBB or RRGGBBAA
		format. The 8 digit form is used only when the alpha component is
		less than 255 (fully opaque).
	*/
	function toHexString():String;

	r = 0.0;
	g = 0.0;
	b = 0.0;
	a = 1.0;
	
	
	/**
		This table defines "named" color values. Each key corresponds
		to a color name, and its value must be a hexadecimal representation
		of the color.
		<p>
		Once put into this table, the named color can be used in the
		Color constructor (and by other methods that require colors
		such as the ProceduralTexture system).
		<p>
		For example:
		<pre>
		Color.names["red"] <- "ff0000";
		local red = Color("red"); // Equivalent to Color("ff0000")
		local blue = Color("blue"); // An error, since "blue" isn't defined
		</pre>
	*/
	static namesTable = {};
}

/**
	A high resolution timer that can be used to profile a
	small section of code.
*/
class Timer
{
	/**
		Reset the timer's internal 'start' counter.
	*/
	function reset();
	/**
		Get the elapsed milliseconds since the timer's construction
		or last reset.
	*/
	function getMilliseconds():Integer;
	/**
		Get the elapsed microseconds since the timer's construction
		or last reset.
	*/
	function getMicroseconds():Integer;
}

/**
	A useful, abstract way of sending generic messages to zero or more recipients.
	<p>
	This is a generic form of the listener design pattern. It basically works
	like this:
	<pre>
	// Set up some listener that will receive the messages.
	mylistener =
	{
		onSomething = function() { ... }
		onSomethingWithArg = function(arg) { ... }
	};
	
	// Create the broadcaster and register the listener. Usually
	// you'll save this somewhere central.
	mb = MessageBroadcaster();
	mb.addListener(mylistener);
	
	// Then, at any time, you can send messages to any registered
	// listeners. The first calls the onSomething handler defined
	// in any listeners.
	mb.broadcastMessage("onSomething");
	
	// This will call the onSomethingWithArg handler in any listeners
	// and will also pass them an argument.
	mb.broadcastMessage("onSomethingWithArg", "Hi!");
	</pre>
*/
class MessageBroadcaster
{
	/**
		Add a listener object to this instance. The listener should implement
		at least one of the events defined in the <code>events</code>
		example structure in order to do any real work.
		<p>
		Although multiple listeners can be registered, a specific instance of
		a listener cannot be registered more than once. Any attempt to do so
		will be ignored.
	*/
	function addListener(listener);
	/**
		Remove a previously registered listener object.
	*/
	function removeListener(listener);
	/**
		Send an event message to all registered listeners (if any).
	*/
	function broadcastMessage(messageName, ...);
}

/**
	The scene is the top-level object that contains all other behavioral and
	renderable objects. You can think of this as a "level" in a game if you
	like (though the analogy is sort of contrived as this is a more generic
	definition). Other things that might be considered a scene: a login screen
	with background elements, a separated inventory screen, the main game
	screen, etc.
	<p>
	A composition may contain multiple scenes in its definition, but only
	one is active at any given time. (Hm, or not, I want to think about this.)
	<p>
	See Ogre::SceneManager
*/
class Scene extends MessageBroadcaster
{
	static FOG_NONE = 0;
	static FOG_EXP = 1;
	static FOG_EXP2 = 2;
	static FOG_LINEAR = 3;

	/**
		No shadows will be cast in the scene.
	*/	
	static SHADOWTYPE_NONE = 0x00;
    /** Stencil shadow technique which renders all shadow volumes as
        a modulation after all the non-transparent areas have been 
        rendered. This technique is considerably less fillrate intensive 
        than the additive stencil shadow approach when there are multiple
        lights, but is not an accurate model. 
    */
	static SHADOWTYPE_STENCIL_MODULATIVE = 0x12;
    /** Stencil shadow technique which renders each light as a separate
        additive pass to the scene. This technique can be very fillrate
        intensive because it requires at least 2 passes of the entire
        scene, more if there are multiple lights. However, it is a more
        accurate model than the modulative stencil approach and this is
        especially apparant when using coloured lights or bump mapping.
    */
	static SHADOWTYPE_STENCIL_ADDITIVE = 0x11;
    /** Texture-based shadow technique which involves a monochrome render-to-texture
        of the shadow caster and a projection of that texture onto the 
        shadow receivers as a modulative pass. 
    */
	static SHADOWTYPE_TEXTURE_MODULATIVE = 0x22;
    /** Texture-based shadow technique which involves a render-to-texture
        of the shadow caster and a projection of that texture onto the 
        shadow receivers, built up per light as additive passes. 
		This technique can be very fillrate intensive because it requires numLights + 2 
		passes of the entire scene. However, it is a more accurate model than the 
		modulative approach and this is especially apparant when using coloured lights 
		or bump mapping.
    */
    static SHADOWTYPE_TEXTURE_ADDITIVE = 0x21;
	/** Texture-based shadow technique which involves a render-to-texture
		of the shadow caster and a projection of that texture on to the shadow
		receivers, with the usage of those shadow textures completely controlled
		by the materials of the receivers.
		This technique is easily the most flexible of all techniques because 
		the material author is in complete control over how the shadows are
		combined with regular rendering. It can perform shadows as accurately
		as SHADOWTYPE_TEXTURE_ADDITIVE but more efficiently because it requires
		less passes. However it also requires more expertise to use, and 
		in almost all cases, shader capable hardware to really use to the full.
		@note The 'additive' part of this mode means that the colour of
		the rendered shadow texture is by default plain black. It does
		not mean it does the adding on your receivers automatically though, how you
		use that result is up to you.
	*/
    static SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED = 0x25;
    /** Texture-based shadow technique which involves a render-to-texture
		of the shadow caster and a projection of that texture on to the shadow
		receivers, with the usage of those shadow textures completely controlled
		by the materials of the receivers.
		This technique is easily the most flexible of all techniques because 
		the material author is in complete control over how the shadows are
		combined with regular rendering. It can perform shadows as accurately
		as SHADOWTYPE_TEXTURE_ADDITIVE but more efficiently because it requires
		less passes. However it also requires more expertise to use, and 
		in almost all cases, shader capable hardware to really use to the full.
		@note The 'modulative' part of this mode means that the colour of
		the rendered shadow texture is by default the 'shadow colour'. It does
		not mean it modulates on your receivers automatically though, how you
		use that result is up to you.
	*/
    static SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED = 0x26;
	
	function getAmbientLight():Color;
	function setAmbientLight(color:Color);

	/**
		Set the material used for the skybox in the scene. The skybox
		is a 6 surface cube with all faces pointing inward (toward the
		camera), and each surface contains a unique part of the "sky".
		The box is constructed so that it always sits "behind" the regular
		scene geometry and thus suitable for texturing the sky and other
		far off details.
		
		@param skyMaterial
			This should refer to a valid cubemap texturing material
			suitable for rendering a skybox. Or, <code>null</code>
			may be passed to disable the skybox.
							
		@param distance
			(Optional) The "distance" that skybox surfaces will be 
			from the camera. Default: 5000
			
		@param drawFirst
			(Optional) If true, this will be drawn before all other
			scene geometry (forcing it to be 'behind' everything else).
			There may be some performance benefits to disabling this
			feature, but higher risk of geometry "peeking through".
			Default: true.
			
		@param orientation
			(Optional) This allows you to change the skybox orientation
			relative to the camera. For instance, to spin or manipulate
			the sky in some way. Default: Quaternion.IDENTITY
	*/
	function setSkyBox(skyMaterial:String, ...);
	
	
	/**
		Set the material used for the skydome in the scene. The skydome
		is much like a sky "box", but with a different texture mapping
		such that 5 faces of box create a more dome like structure. This
		is useful in environments where the floor is always present and
		you cannot see below the horizon (as there is nothing defined
		below the viewer).
		<p>
		Sky domes work well with repeating textures such as clouds, and
		can also be combined with a skybox to provide more multiple layers
		of sky effects.
		
		@param skyMaterial
			This should refer to a valid material to use when rendering
			the skydome. Unlike a skybox, this should not be a "cubemap"
			textured material. Or, <code>null</code> may be passed to
			disable the skydome.
			
		@param curvature
			(Optional) The amount of curvature the generated mapping
			will have. Values between 2 and 65 are generally pretty good,
			with higher values providing more curvature and a smoother
			effect (at the cost of more distortion at a distance).
			Default: 10
							
		@param distance
			(Optional) The "distance" that the dome surfaces will be 
			from the camera. Default: 4000
			
		@param drawFirst
			(Optional) If true, this will be drawn before all other
			scene geometry (forcing it to be 'behind' everything else).
			There may be some performance benefits to disabling this
			feature, but higher risk of geometry "peeking through".
			Default: true.
			
		@param orientation
			(Optional) This allows you to change the skydome's orientation
			relative to the camera. For instance, to spin or manipulate
			the sky in some way. Default: Quaternion.IDENTITY
	*/
	function setSkyDome(skyMaterial:String, ...);


	/**
		Set the material used for a "sky plane". This creates a planar
		object that is always a fixed distance from any camera.
		
		@param skyMaterial
			This should refer to a valid material to use when rendering
			the skydome. Unlike a skybox, this should  be a "normal"
			textured material (i.e. not a cubemap). Or, <code>null</code>
			may be passed to disable the sky plane.
			
		@param normal
			The normal indicating the "direction" of the plane. This and
			the <tt>point</tt> define the equation of the plane.
			
		@param point
			A point through which the plane should intersect. This and
			the <tt>normal</tt> define the equation of the plane.
		
		@param scale
			(Optional) The size of the plane in world coordinates. The equation
			of the plane (i.e. how far "up"/"away" it is, will probably affect the
			desired value here. Default: 1000.0
			
		@param tiling
			(Optional) How many times to wrap the texture coordinates across
			the surface of the plane. Default: 10.0
			
		@param drawFirst
			(Optional) If true, this will be drawn before all other
			scene geometry (forcing it to be 'behind' everything else).
			There may be some performance benefits to disabling this
			feature, but higher risk of geometry "peeking through".
			Default: true.

		@param bow
			(Optional) This will warp the plane slightly (achieving a more
			"dome-like" appearance, but which is often more compatible with
			fog. A value of 0 will give a perfectly flat plane. Default: 0
							
		@param xsegments
			(Optional) The number of horizontal segments to use when generating
			the plane. Useful for bowing and/or increased tesselation for 
			per-vertex effects. Default: 1 
			
		@param ysegments
			(Optional) The number of vertical segments to use when generating
			the plane. Useful for bowing and/or increased tesselation for 
			per-vertex effects. Default: 1 
	*/
	function setSkyPlane(skyMaterial:String, normal:Vector3, point:Vector3, ...);

	/**
		Set the fog environmental parameters for the current scene. The
		fog settings will apply to ALL rendered objects in the scene unless
		they have a material that happens to have its own fog settings.

		@param mode
				Set the fog mode. The valid options here are:
				<dl>
				<dt>FOG_NONE</dt>
				<dd>Disable fog entirely. The rest of the options are ignored.</dd>
				<dt>FOG_EXP</dt>
				<dd>Fog density increases exponentially from the camera</dd>
				<dt>FOG_EXP2</dt>
				<dd>Fog density increases at the square of the exponential distance.</dd>
				<dt>FOG_LINEAR</dt>
				<dd>Fog density increases linearly from the start to end distances.</dd>
				</dl>
		@param color
				The color for the fog. Usually this is set to the viewport background
				color or a primary color in any skybox you have set up.
		@param density
				An exponential fog density parameter. Default: 0.001
		@param start
				Linear fog start distance. Default: 0.0
		@param end
				Linear fog end distance. Default: 1.0
	*/
	function setFog(mode:Integer, color:Color, density:Float, start:Float, end:Float);
	/**
		Get the current fog mode.
		@see setFog()
	*/
	function getFogMode():Integer;
	/**
		Get the current fog color.
		@see setFog()
	*/
	function getFogColor():Color;
	/**
		Get the current fog start distance.
		@see setFog()
	*/
	function getFogStart():Float;
	/**
		Get the current fog end distance.
		@see setFog()
	*/
	function getFogEnd():Float;
	/**
		Get the current fog density.
		@see setFog()
	*/
	function getFogDensity():Float;
	
	/**
		Set the shadow generation algorithm type. This must be
		one of the SHADOWTYPE_* definitions, and must be explicitly
		set to see shadow (the default is SHADOWTYPE_NONE).
	*/
	function setShadowTechnique(shadowType:Integer);
	
	/**
		Some shadow techniques requires a max extrusion distance
		(i.e. stencils), whereas others simply prefer it for optimization
		reasons. Generally speaking, this is the maximum distance
		that shadows will "reach" from the shadow caster. Setting
		this to 0 will cause an infinite distance to attempt to be
		used, but this is not supported for all techniques and all
		cards. Default: 0.0
	*/
	function setShadowFarDistance(distance:Float);
	
	/**
		Set the size of the texture used for texture based shadowing
		techniques. This must be a power of 2. Higher resolutions
		will provide better results, but at the cost of more video
		memory. The default size is 512.
	*/
	function setShadowTextureSize(size:Integer);

	/**
		Return true if the scene node with the given name exists
		in the current scene.
	*/
	function hasSceneNode(name:String):Boolean;
	/**
		Get the current scene node given its name. If it doesn't exist,
		then an exception is thrown.
	*/
	function getSceneNode(name:String):SceneNode;
	/**
		Create a new unattached scene node. This node must be attached
		to another node that is in the scene (either the root node, or
		one that is attached to the root node indirectly) in order to
		be visible.
		
		@param name (Optional) A name for this node. If not specified,
				then a name will be generated.
		
		@throws If a name is specified, and refers to an already existing
			node.
	*/
	function createSceneNode(...):SceneNode;
	/**
		Get the root scene node for this scene.
	*/
	function getRootSceneNode():SceneNode;

	/**
		Does the entity with the given name exist in the scene?
	*/
	function hasEntity(name:String):Boolean;
	/**
		Does the camera with the given name exist in the scene?
	*/
	function hasCamera(name:String):Boolean;	
	/**
		Does the light with the given name exist in the scene?
	*/
	function hasLight(name:String):Boolean;
	/**
		Does the sound emitter with the given name exist in the scene?
	*/
	function hasSoundEmitter(name:String):Boolean;
	/**
		Does the decal with the given name exist in the scene?
	*/
	function hasDecal(name:String):Boolean;
	/**
		Does the text board with the given name exist in the scene?
	*/
	function hasTextBoard(name:String):Boolean;
	/**
		Does the texture projector with the given name exist in the scene?
	*/
	function hasTextureProjector(name:String):Boolean;

	/**
		Get an entity using its name. If an entity does not exist with the
		given name, an error is thrown.
	*/
	function getEntity(name:String):Entity;
	/**
		Get a camera using its name. If a camera does not exist with the
		given name, an error is thrown.
	*/
	function getCamera(name:String):Camera;
	/**
		Get a light using its name. If a light does not exist with the
		given name, an error is thrown.
	*/
	function getLight(name:String):Light;
	/**
		Get a particle system using its name. If the particle system
		does not exist with the given name, an error is thrown.
	*/
	function getParticleSystem(name:String):ParticleSystem;
	/**
		Get a sound emitter using its name. If the sound emitter does
		not exist with the given name, an error is thrown.
	*/
	function getSoundEmitter(name:String):SoundEmitter;
	/**
		Get a decal using its name. If the decal does not exist with
		the given name, an error is thrown.
	*/
	function getDecal(name:String):Decal;
	/**
		Get a text board using its name. If it does not exist,
		an error is thrown.
	*/
	function getTextBoard(name:String):TextBoard;
	/**
		Get a texture projector using its name. If it does not exist,
		an error is thrown.
	*/
	function getTextureProjector(name:String):TextureProjector;

	/**
		Create an entity in the scene. Entities are the primary
		form of visible and objects. They are instances
		of a specific mesh. Entities, like any other movable object,
		must have a unique name.

		@param name
			The name to use for this entity. (Must be unique.)
		@param mesh
			The name of a mesh to use as the entity's visual
			representation.
			
		@return A new entity instance.
		
		@throws If the entity already exists or is an invalid name.
		@throws If the mesh is invalid.
	*/
	function createEntity(name:String, mesh:String):Entity;

	/**
		Create a camera. 
		
		@param name
			The name of the camera to create. (Must be unique.)

		@return The new camera instance.
	*/
	function createCamera(name:String):Camera;
	
	/**
		Create a new light. The new light is by default, an Omni
		type light (you can use the methods on the light to change
		its behavior).

		@return The new omni light.
	*/
	function createLight(name:String):Light;

	/**
		Create a new particle system. The new particle system must
		be named uniquely and reference an existing particle
		system definition script (.particle file). Like any other
		movable object, it must be attached to a scene node before
		it can be seen.
		
		@param name The name of the particle system.
		
		@param template The particle system template/definition.

		@return The new particle system instance.
	*/
	function createParticleSystem(name:String,
					template:String):ParticleSystem;
					
	/**
		Create a new sound emitter set to play a specific sound.
		The emitter must be uniquely named (like any movable object),
		and will load the specified sound (if possible), but will
		NOT start playing until explicitly told to do so.
		
		@param name The name of the sound emitter object to create.
		
		@param sound (Optional) The name of a sound resource (filename
			usually) to set by default.
			
		@return The new sound emitter.
	*/
	function createSoundEmitter(name:String, sound:String):SoundEmitter;
	
	/**
		Create a new decal, given a material name and a size. The decal
		defaults to pointing directly "up" along the +Y axis, and you must
		explicitly set its position and orientation using Decal.setPosition()
		and/or Decal.setOrientation() to change it. (It must also still be
		attached to a scene node like any movable object.)
		<p>
		Note: Remember, decal materials almost always require a <tt>depth_bias</tt>
		to avoid flickering against coplanar geometry.
	*/
	function createDecal(name:String, material:String, width:Float, height:Float):Decal;
	
	/**
		Create a new TextBoard using the specified font and line height.
		Unless you specify a bit of text as the last parameter in this
		call, the new text board will exist, but have no text, and thus
		be invisible.
		<p>
		If the font does not exist, an error is thrown.
		<p>
		The line height is in world units. {@see TextBoard#setLineHeight}
	*/
	function createTextBoard(name:String, fontName:String, lineHeight:Float, ...):TextBoard;
	
	/**
		Create a texture projector in the scene using a specified texture.
		By default, the projector uses a perspective projection, so if you
		want an orthographic one (more suitable to projecting feedback onto
		flat surfaces), you must set it explicitly after creating it.
		<p>
		@param name The name for the projector in the scene.
		@param textureName The name of the texture to project.
	*/
	function createTextureProjector(name:String, textureName:String):TextureProjector;
	
	
	/**
		Create a paged geometry instance that will manage creation and deletion
		of geometry objects on the fly in response to camera movement. Beyond
		naming and setting up the extents in which the geometry will be managed,
		the heart of paged geometry is in the "LOD"/detail system(s) used to render
		the objects loaded.
		<p>
		A quick overview of how this works is as follows: The paged geometry is
		capable of loading a specific type of content (identified by the <tt>type</tt>
		parameter) using a predefined algorithm and/or dataset. The paged
		geometry instance then uses its "Detail table" to select a representation
		for that geometry based on the distance to the camera. This detail table
		can have multiple ranges, or just one, and it will automatically switch
		between them at runtime based on the distance to the camera. Various
		techniques can be set for each entry in the detail table, each with 
		varying performance characteristics (and memory/speed tradeoffs).
		<p>
		Each detail levels can be one of the following:
		<dl>
		<dt>Batch</dt>
		<dd>This uses Ogre's StaticGeometry to bundle entities
			into various geometry buckets for efficient rendering and reducing
			batch counts.</dd>
		<dt>Impostor</dt>
		<dd>This uses an "impostoring" technique to render geometry from several
			angles into a single texture and then create a billboard representation
			of the object using those snapshot images. This is useful for rendering
			very complicated objects (such as trees) at a distance. Currently this
			implementation does not handle "looking down" very well so it's assumed
			the camera will be located generally horizontally (i.e. in the X/Z plane)
			relative to the object(s) being rendered.</dd>
		<dt>Grass</dt>
		<dd>The grass method is a simple stub used by the "grass" geometry loader
			to hold the grass meshes generated programattically.</dd> 
		</dl>
		<p>
		Let's look at an example:
		<pre>
		local geom = _scene.createPagedGeometry("trees", bounds, "PagedTree2D",
					{
						"Batch" => [ 200, 50 ]
						"Impostor" => [ 400 ]
					});
		</pre>
		<p>
		In the above example, a single paged geometry instance is created, 
		named "trees". It uses a previously defined bounding box, and the
		"Tree2D" loader type.
		
	*/
	function createPagedGeometry(name:String, bounds:AxisAlignedBox, type:String,
					detailConfig:Table):PagedGeometry;
	
	/**
		Look up an existing paged geometry instance. Unlike the MovableObject
		derived lookup functions, this will not throw an error if it does not
		exist, it simply returns <tt>null</tt> if it's not found, or the
		instance if it is.
	*/
	function getPagedGeometry(name:String):PagedGeometry;
	
	/**
		Set the object that will be used for 3D positional audio
		listening. In order to use positional audio, you must:
		<ol>
		<li>Attach every sound emitter that will be positional to
		a scene node. (So the emitter can have a position)</li>
		<li>Set a non-null movable object as the "listener" using
		this method.</li>
		</ol>
		<p>
		Failure to do one or both of the above items will result
		in regular 2d sound without any 3d panning or volume effects.
		<p>
		You can combine the techniques (i.e. have both 3D positional
		audio as well as "regular" audio) by either never attaching
		the "regular" emitter(s) to a scene node (they will still
		play sounds, it will just not be positional) or attaching it
		to the same scene node as the listener (i.e. it comes from
		exactly the same place as the listener).
		<p>
		Usually, the active camera is used as the listener in a
		3D audio configuration (but it doesn't have to be).
		<p>
		Set this to <code>null</code> to disable 3D sound entirely
		(this is the default).
		
		@param listenerObject If non-null, this object will be used
			for 3D audio (usually the active camera). Otherwise,
			3D audio will be disabled.
	*/
	function setSoundListener(listenerObject:MovableObject);
	
	/**
		Get the current scene visibility mask.
	*/
	function getVisibilityMask():Integer;
	
	/**
		Set the scene's visibility mask. This value is bit-wise
		AND'ed with the visibility flags of each potentially
		visible movable object, and only those that are
		non-zero afterward are rendered.
		<p>
		The default value for this is 0xFFFFFFFF (all bits visible).
		
		@param flags The new visibility mask.
	*/
	function setVisibilityMask(flags:Integer);

	/**
		Configure the scene's terrain using a TerrainSceneManager
		configuration file. Full documentation of the terrain
		configuration file can be found in the Ogre documentation
		(or in the terrain samples of the SDK).
		<p>
		Setting this enables the terrain load events for the
		scene object, allowing you to hook into the paged
		geometry loading mechanics of the TerrainSceneManager
		and provide seamless scene construction and destruction.
		<p>
		This will replace any existing terrain configuration that
		may be in place.
		
		@param terrainConfigFile The name of a configuration file
			available in one of the loaded compositions that contains
			the terrain configuration to use. You may also set this
			to "" to destroy the terrain completely without loading
			another.
	*/
	function setWorldGeometry(terrainConfigFile:String);
	
	/**
		Set the query and visibility flags to use for the generated
		world geometry objects. This only applies to newly created
		geometry objects, so it must be called prior to
		setWorldGeometry().
		<p>
		This value will be retained until the player is closed and
		is not reset upon loading up a new terrain configuration via
		setWorldGeometry().
		
		@param queryFlags The value to use for query flags, or
			if it is <tt>null</tt> use the default for all MovableObjects.
		@param visibilityFlags The value to use for visibility flags, or
			if it is <tt>null</tt> use the default for all MovableObjects.
	*/
	function setTerrainFlags(queryFlags:Integer, visibilityFlags:Integer);
	
	/**
		Get the scale used in the terrain configuration file. This
		scale represents the size of a single page of terrain. If
		no terrain has been initialized the return value is undefined.
	*/
	function getTerrainScale():Vector3;
	
	/**
		Get a page index pair for the given world coordinate. This will
		calculate the corresponding terrain page index in each X/Z axis
		and return it in a table with "x" and "z" members. If no terrain
		has been set, this will return null.
		@param pos The world position to calculate terrain page indexes for.
	*/
	function getTerrainPageIndex(pos:Vector3):Table
	
	/**
		Create a new scene component (a predefined subscene) based on
		a CSM resource name (e.g. "simpleVillage"). The CSM definition
		is loaded via the Ogre resource loading mechanisms and is
		automatically exposed for use if present. The component must
		be uniquely named as well (this name is used to destroy it in
		destroySceneComponent).
		<p>
		Unless a "root" node is provided, it will automatically
		attach all objects to a new -detached- SceneNode (make sure
		you attach it somewhere or it won't be visible), otherwise it
		will attach all instantiated objects and children to the
		provided node instead.

		@param componentName The name (or ID in CSM terms) of the CSM
				definition to instantiate into this scene. This is the 
				id of the corresponding Component element in an XML file.

		@param topNode If non-null, the scene contents should be added
				to this node, rather than creating a detached one by default.
				Note: you must specify this if you're using CSM keep-on-floor
				semantics, or you will not have accurate world locations and
				thus the objects cannot locate the floor properly.
		
		@param vars This allows variable expansion in the assets referenced
			by the CSM resource. (e.g. "ATS" = "ATS-Bremen" will cause
			Bremen tiles to be used in a building definition).
			
		@param addQueryFlags (Optional) If present, this integer specifies an 
			additive query flags override. These flags will be added
			to any query flags already specified (either explicitly
			or via defaults) when creating movable objects. If 0, this
			effectively does nothing. Default: 0
			
		@param removeQueryFlags (Optional) If present, this integer will remove
			any query flags from created movable objects regardless of
			their specified settings (either explicitly or via defaults).
			Adding flags takes precedence over removing them, (i.e. if
			the same bit is in both the add and remove set, it will be
			added, not removed). If 0, this effectively does nothing.
			Default: 0   

		@return The "root" SceneNode with the various created parts of the scene
			instantiated and positioned correctly.
	*/
	function createComponentInstance(componentName:String,
			topNode:SceneNode, interactiveMode:Boolean, 
			vars:Table, ...):SceneNode;
			
	/**
		Set up CSM component instantiation defaults. These are used when
		calling Scene::createComponentInstance(), and allow the specific
		type of objects created via that call to be tweaked. For instance,
		you can change the default query flags for all Entity objects
		(i.e. those that don't specify exactly).
		<p>
		This also controls PagedCSM instantiations which are somewhat
		"asynchronous" in that they are bound to the camera's position.
		Thus, it is advised to set this only one (at initialization time)
		and leave it for the lifetime of that scene.
		<p>
		The table can have the following elements:
		<dl>
		<dt>visibilityFlags</dt>
		<dd>This is a string specifying a "flag modification" specification.
		    This can be a single integer value representing the bits
		    that should be enabled (e.g. 16) or an "expression" that combines
		    these values in an additive/subtractive manner. To accomplish this,
		    prefix one or more numbers with a '+' for additive (i.e. add to
		    the "current" flag value) or '-' for subtractive (i.e. remove the
		    corresponding flags). For example, "-1+4+1024" would remove the
		    "1" bit (if set), and add the "4" and "1024" bits. The latter
		    two could have been combined numerically to +1028 but it is often
		    advantageous to explicitly list which bits are enabled when humans
		    are concerned.</dd>
		<dt>queryFlags</dt>
		<dd>This is exactly like <tt>visibilityFlags</tt> but pertains to the
			query flags member of any created objects.</dd>
		<dt>castShadows</dt>
		<dd>This is a boolean (1 for "on", 0 for "off") indicating whether the
			object should cast shadows. The default is "on" for anything created
			via CSM (unless this has been explicitly changed).</dd>
		<dt><i>(Name of a CSM type tag)</i></dt>
		<dd>Any other term is assumed to be a "type customization" entry which
			is a nested state definition that is exactly like this table
			specification (minus the nesting), which allows you to change
			which values are used for specific node types. For instance, you
			could specify <tt>["Entity"]={queryFlags="+16"}</tt> to change
			the default query flags for only "Entity" nodes.</dd> 
		</dl>
	*/
	function setComponentDefaults(defaults:Table);

	// Ph. II
//	function createGrid(name:String):Grid;
//	function createBillboard(name:String):Sprite3D;
	// Ph. III
//	function createPointCloud(name:String):PointCloud;
//	function createCurve(name:String):Curve;
//	function createPlace(name:String):Place;

	/**
		Perform a ray scene query. This will cast a ray from the given 
		origin in the specified direction and return a sorted list of
		intersections with the scene's geometry (if any).
		<p>
		Note: If neither <tt>checkFrontFaces</tt> nor <tt>checkBackFaces</tt>
		is true (both default to false), this performs bounding box 
		intersection only! This is often not very accurate and useful
		only for quick triviality checks.
		<p>
		The returned array contains tables with the following members:
		<dl>
		<dt>t (float)</dt>
		<dd>The distance along the ray, from its origin, that the intersection
			was found.</dd>
		<dt>pos (Vector3)</dt>
		<dd>This is the point of intersection (or at least the closest
			point that can be determined quickly).
		<dt>normal (Vector3)</dt>
		<dd>The estimated normal at the point of intersection based on
		    the polygon intersected. Note this may not be the "real"
		    normal, especially if the mesh has normal data (as normals
		    are not interpolated), but the normal of the polygon's face.</dd>
		<dt>object (MovableObject or null)</dt>
		<dd>This is the object containing the intersected polygon, or if
			it is a terrain intersection, it will be <code>null</code></dd>
		</dl>
		
		@param rayStart The origin of the ray to cast.
		@param rayDir The directional vector of the ray to cast.
		@param queryMask (Optional) If present, only those objects matching
			the mask will be considered.
		@param checkFrontFaces (Optional) If present and true, the
			front faces of any entities found will be checked for
			intersection. 
		@param checkBackFaces (Optional) If present and true, the
			backfaces of any entities found will also be tested and
			included as intersection points.
		@param maxHits (Optional) If present, this will limit the query to the first N
			hits that are found (in increasing distance from the camera).
		@param ignoreNode (Optional) If present and non-null, all movables
			that are children of the given scene node (either directly or
			indirectly) will not be considered as valid candidates. Default
			is <tt>null</tt>.
			
		@return An array of "records" as described above.
	*/
	function rayQuery(rayStart:Vector3, rayDir:Vector3, queryMask:Integer,
			checkFrontFaces:Boolean, checkBackFaces:Boolean,
			maxResults:Integer, ignoreNode:SceneNode):Array;
			
	/**
		Perform a volumetric "box" query, returning any movable object
		with a matching query flag settings and whose bounding box
		intersects the given box. It will return the movable objects
		that were intersected (if any).
		
		@param aab An AxisAlignedBox that defines the query volume.
		@param queryMask (Optional) A bitwise mask to use to determine query
			inclusion. If not provided, it will default to 0xFFFFFFFF.
		@return An array of MovableObjects matching the query (this array
			may very well be empty if no objects match).
	*/
	function boxQuery(aab:AxisAlignedBox, queryMask:Integer):Array;

	/**
		Performs a continuous box-sweep collision along a path and returns the furhest distance
		that can be moved before a collision occurs.

		@param box The half-extents of the box to sweep
		@param startPos The starting position of the path
		@param endPos The destination position of the path
		@param mask A bitwise mask to use determine inclusion of objects into the test, if not provided it will default to 0xFFFFFFFF
		@param filter Whether or not to filter out 'floor' polygons (Polygons that point upwards). Default is false
		@param threshold The threshold to use when filtering out floor polygons. Default is 0.5	

		@return The furthest distance along the path of movement before a collision occurred. If there was no collision than 1.0 is returned.
	*/
	function sweepBox(extents:Vector3, startPos:Vector3, endPos:Vector3, mask:Integer, filter:Bool, threshold:Float):Float;

	/**
		Set the wind direction for animated grasses. If the user's
		graphics system does not support vertex shading this is
		ignored.
	*/
//	function setGrassWindDirection(dir:Vector3);
	
	/**
		Gets the current wind direction used by this paged geometry
		instance.
	*/
//	function getGrassWindDirection():Vector3;
	
	/**
		Set a density modification factor for grassed created using
		the paged geometry system. This controls the overall
		density of all grasses created. This is multiplied by the grass
		loader's normally calculated density to obtain the "real" density.
		<p>
		For example, if a grass layer layer had density 0.4, and this
		value was 0.5, the final density would be 0.2 ( = 0.4 * 0.5 ).
		That final density is then used to calculate whether grass geometry
		is present at a given location.
		<p>
		This is effectively allows generated grass usage to scale
		based on a user's preferences/performance settings. The default
		value is 1.0.
		
		@param density A value between 0.0 and 1.0 (inclusive).
	*/
	function setGrassDensityFactor(density:Float);
	
	/**
		Get the overal density modification factor for this loader.
		@see #setDensityFactor().
	*/
	function getGrassDensityFactor():Float;
	
	
	/**
		Set an extra buffering distance to be used by the terrain
		engine when loading in terrain pages. This distance is added
		to the primary camera's far clip distance in order to find
		the final radius for loaded terrain pages.
		<p>
		Setting this to greater than 0 (the default) will result in
		pages outside the rendering distance being loaded, which is
		useful when you require a certain radius around a loaded
		object to be defined. (Dynamic grasses and trees come to mind.)
		
		@param dist The distance (in world space) to add to camera
		            rendering distance.
	*/
	function setTerrainBufferDistance(dist:Float);
	
	/**
		Get the current extra terrain buffering distance.
		@see #setTerrainBufferDistance
	*/
	function getTerrainBufferDistance();

	/**
		Check if the terrain underneath the specified circular region
		has been loaded and is ready to have height values queried from
		it. This method quickly lets you determine if there is valid
		height data within the given region. This operates at the page
		level and if the region extends into "invalid" pages (i.e.
		negative or beyond the configured maximum), it will not prevent
		it from returning true if all the other pages are ready.
		<p>
		If no terrain has been configured, this always returns true.
		
		@param x The world space x coordinate.
		@param z The world space z coordinate.
		@param radius The world space radius to check for readiness.
	*/
	function isTerrainReady(x:Float, z:Float, radius:Float);

	static events =
	{
		/**
			A terrain page (set of terrain tiles) has fully loaded. This
			is called after the last tile in the given page has been
			loaded.
			<p>
			You should probably not initialize extra scene elements in
			this event, but rather in onTerrainTileLoaded(), as that
			provides a much finer grained (and staggered) opportunity
			for loading scene data.
			<p>
			The page coordinates are indexes into the virtual 2D "page map" 
			of all pages (as defined in the terrain configuration file).
			
			@param pageX The X axis page coordinate of the page.
			@param pageZ The Z axis page coordinate of the page.
			@param bounds The bounding box for this tile.
		*/
		onTerrainPageLoaded = function(pageX:Integer, pageZ:Integer,
					bounds:AxisAlignedBox){}
		
		/**
			A terrain page (set of terrain tiles) has fully unloaded. This
			is called after the last tile in the given page has been
			unloaded.
			<p>
			If you allocated any scene objects during a page loaded event,
			you should probably deallocate them in either a tile unloaded
			event or as a last resort, in this method.
			<p>
			The page coordinates are indexes into the virtual 2D "page map" 
			of all pages (as defined in the terrain configuration file).
			
			@param pageX The X axis page coordinate of the page.
			@param pageZ The Z axis page coordinate of the page.
			@param bounds The bounding box for this page.
		*/
		onTerrainPageUnloaded = function(pageX:Integer, pageZ:Integer,
					bounds:AxisAlignedBox){}
		
		/**
			A specific tile within a terrain page has been loaded.
			This is like onTerrainPageLoaded(), but at the tile level
			not the page level. The number of tiles per page is
			set via the terrain configuration file.
			<p>
			You should use this event to create scene elements
			such as entities and static geometry that are defined
			for this part of the world.
			
			@param pageX The X axis page coordinate of the page
					containing the tile.
			@param pageZ The Z axis page coordinate of the page
					containing the tile.
			@param tileX The X axis tile coordinate within the
					containing page.
			@param tileZ The Z axis tile coordinate within the
					containing page.
			@param bounds The bounding box for this tile.
		*/
		onTerrainTileLoaded = function(pageX:Integer, pageZ:Integer,
			tileX:Integer, tileZ:Integer, bounds:AxisAlignedBox){}

		/**
			A specific tile within a terrain page has been unloaded.
			This is like onTerrainPageUnloaded(), but at the tile level
			not the page level. The number of tiles per page is
			set via the terrain configuration file.
			<p>
			You should use this event to destroy scene elements
			for this part of the world since they will no longer
			be visible.
			
			@param pageX The X axis page coordinate of the page
					containing the tile.
			@param pageZ The Z axis page coordinate of the page
					containing the tile.
			@param tileX The X axis tile coordinate within the
					containing page.
			@param tileZ The Z axis tile coordinate within the
					containing page.
			@param bounds The bounding box for this tile.
		*/
		onTerrainTileUnloaded = function(pageX:Integer, pageZ:Integer,
			tileX:Integer, tileZ:Integer, bounds:AxisAlignedBox){}
	};



}


/**
	Nodes are used to organize the scene into a transformed hierarchy. This
	base class does not actually provide the "data" used by the nodes, it's 
	simply the structure for transforming them. More specific derived classes
	(e.g. {@link SceneNode}) carry the functionality to attach objects and
	renderable elements.
*/
class Node
{
	static TS_LOCAL = 0;	// Local coordinate space
	static TS_PARENT = 1;	// Parent's coordinate space
	static TS_WORLD = 2;	// World coordinate space
	
	/**
		Get the unique name of this scene node object. Note
		that this value is assigned when the object is created,
		and remains read-only for the life of the object.
	*/
	function getName():String;

	/**
		Get the local positional transform component.
	*/
	function getPosition():Vector3;
	
	/**
		Set the local positional transform component.
	*/
	function setPosition(newpos:Vector3);
	
	/**
		Get the node's local orientation.
	*/
	function getOrientation():Quaternion;
	/**
		Set the node's local orientation.
	*/
	function setOrientation(orientation:Quaternion);
	/**
		Reset the node's local orientation. This aligns the
		local axes to the world axes (i.e. no rotation).
	*/
	function resetOrientation();
	/**
		Get whether orientation changes in the parent are reflected
		in this object.
	*/
	function getInheritOrientation();
	/**
		Set whether orientation changes in the parent object will be
		passed along to this object.
		<p>
		Setting this to false will cause orientations to not be passed
		on to this object whenever the parent changes. This allows a
		"position-only" heirarchy that is useful in some situations. The
		default is to inherit parent orienation changes.
	*/
	function setInheritOrientation(value:Boolean);
	
	/**
		Get whether the parent's scale changes affect this object.
	*/
	function getInheritScale();
	/**
		Set whether the parent's scale changes affect this object.
		<p>
		Setting this to false will cause the parent's scale to not
		affect the scale of this object. Default is to inherit the
		parent's scale.
	*/
	function setInheritScale(value:Boolean);
	
	/**
		Get the current local scale factor.
	*/
	function getScale():Vector3;
	/**
		Set the current local scale factor.
	*/
	function setScale(newscale:Vector3);

	/**
		Perform a relative translation by the given amount.
		<p>
		Translation can be either a single Vector3 instance or
		3 floats (x, y, z). And there can optionally be a final
		argument indicating the transform space (default: TS_PARENT).
	*/
	function translate(...);
	/**
		Scale the object by the given amount.
		<p>
		Scale can be either a single Vector3 instance or an X,Y,Z
		float triplet.
	*/
	function scale(...);
	/**
		Rotate an object.
		<p>
		The rotation can be specified in several ways:
		<ul>
		<li>Vector3 (axis of rotation), float (angle to rotate, in radians)</li>
		<li>Quaternion (rotation)</li>
		</ul>
		Each form can also optionally include a transform mode (<tt>TS_*</tt>)
		as additional last argument. If omitted, the default is TS_LOCAL.
	*/
	function rotate(...);
	
	/**
		Rotate around the local X axis.
		@param rads Radians to rotate around the local axis.
		@param relativeTo (Optional) What space the transform is relative to.
				Default: TS_LOCAL
	*/
	function pitch(rads, ...);
	
	/**
		Rotate around the local Y axis.
		@param rads Radians to rotate around the local axis.
		@param relativeTo (Optional) What space the transform is relative to.
				Default: TS_LOCAL
	*/
	function yaw(rads, ...);
	
	/**
		Rotate around the local Z axis.
		@param rads Radians to rotate around the local axis.
		@param relativeTo (Optional) What space the transform is relative to.
				Default: TS_LOCAL
	*/
	function roll(rads, ...);
	
	/**
		Set a fixed yaw axis. This is disabled by default, but can be
		enabled to change the mechanics of the yaw operation. The normal
		behavior of yaw is to rotate around the local Y axis, but if
		this is enabled, it will always rotate around the world-space
		Y axis. This is useful in camera-style looking or auto-tracking
		situations to keep the node "upright".
	*/
	function setFixedYawAxis(enabled:Boolean);

	/**
		Orient the node so its negative Z axis points toward the given
		point.
	*/
	function lookAt(pos:Vector3);

	/**
		Get the node's absolute world orientation.
	*/
	function getWorldOrientation():Quaternion;
	
	/**
		Get the node's absolute world position.
	*/
	function getWorldPosition():Vector3;
	
	/**
		Get the parent node for this node. If there isn't one set,
		<code>null</code> is returned.
	*/
	function getParent():Node;

	/**
		Add a detached child node as a child of this node.
		
		@param child
			The child object to add.
			
		@throws If the child node is already attached to another node.
	*/
	function addChild(child:Node);
	
	/**
		Get the Nth child object attached to this scene object. If
		the index does not reference a valid child (less than 0 or
		greater than getChildCount()), <code>null</code> is returned.
		
		@param index
			The 0-based index into the number of children.
	*/
	function getChild(index:Integer):Node;
	
	/**
		Get the current number of children attached to this node. If
		there are none, 0 is returned.
	*/
	function numChildren():Integer;
	
	/**
		Remove a specific child from this object. You can specify
		either a node name or a child's numeric index (&lt; numChildren()).
	*/
	function removeChild(...);
	
	/**
		Get an array of any children nodes attached to this node.
		If there are none, an empty array ([]) is returned.
	*/
	function getChildren():Array;
}

/**
	Scene nodes allow MovableObjects to be attached to them for purposes
	of rendering. They are also optimized by the current scene manager in
	use for efficient scene traversal and visibility determination.
	<p>
	Every scene has exactly one root scene node (see Scene::getRootSceneNode())
	which all visible objects must be attached to (either directly or
	indirectly) in order to be seen.
*/
class SceneNode extends Node
{
	/**
		Create and add a new scene node. The new scene node will
		be added as a child of this node. If a name is supplied,
		it must be unique.
	*/
	function createChildSceneNode(...):SceneNode;
	
	/**
		Toggle visiblity of the node's bounding box. Note, that depending
		on the type of scene manager currently being used, the bounding
		box may or may not include child nodes (and in fact, our default
		implementation does not). This is primarily for debugging purposes.
	*/
	function showBoundingBox(value:Boolean);
	
	
	/**
		Attach a movable object to this scene node. Once attached, the
		movable object is transformed according to the scene node, and
		is included in the possible render set (assuming the scene node
		is itself visible). An object can only be attached to a single
		scene node at a time.
	*/
	function attachObject(object:MovableObject);
	
	/**
		Detach an object from the scene node. The object must have been
		previously attached using attachObject().
	*/
	function detachObject(object:MovableObject);
	
	/**
		Get an array of the currently attached MovableObjects for this
		scene node. If nothing is attached, this returns an empty array
		(i.e. []).
	*/
	function getAttachedObjects():Array;
	
	/**
		Set the automatic "keep on floor" mask that will force this
		node to place itself on the surface of the nearest mesh found
		directly below or above this scene node (+/- UNIT_Y axis).
		<p>
		Set this to 0 to disable keep on floor for this node (the
		default).
		
		@param mask The mask to use when performing a ray query to
			look for floor.
	*/
	function setFloorAnchorMask(mask:Integer);
	
	/**
		If set via setFloorAnchorMask(), this will return the mask
		used to glue this node to the "floor". The default is 0,
		which is to not perform this logic.
	*/
	function getFloorAnchorMask():Integer;
	
	/**
		If a floor anchor mask has been set, this can be used to
		change the final Y-axis value used by the floor anchor for
		this node. A positive number will cause the final point to
		be above the found floor intersection, while a negative one
		will be "sunk in". The default when first setting the anchor
		is 0.0 (and will not be remembered if the anchor is removed
		and then reattached).
		<p>
		If this is called while the floor anchor mask is 0 (i.e.
		disabled), this will do nothing.
		
		@param offset The Y-axis offset value for the final
				intersection point. 
	*/
	function setFloorOffset(offset:Float);
	
	/**
		Get the floor offset value used by this node's floor anchor.
		If a floor anchor mask has not been set (the default) this
		will return <tt>null</tt> instead.
		
		@return The offset value, or <tt>null</tt> if no anchor has
			been set for this node.
	*/
	function getFloorOffset();
	
	
	/**
		This will recursively destroy the scene node and its children
		(both nodes and attached movable objects).
	*/
	function destroy();
}

/**
	This is the abstract base class for all attachable scene objects.
	This has various methods for accessing the parent/attached scene
	node and various other things. It does not actually render anything
	itself, relying on derived classes such as Entity and Light to
	provide that functionality.
*/
class MovableObject
{
	/**
		Get the parent node for this object. This is the generic
		form of node parent, and can be either a SceneNode or some
		other derived class (such as a bone attachment).
	*/
	function getParentNode():Node;
	
	/**
		Get the parent scene node for this object. If it hasn't been
		attached yet, this will be <code>null</code>.
	*/
	function getParentSceneNode():SceneNode;
	
	/**
		Get the derived class movable type name.
	*/
	function getMovableType():String;
	
	/**
		Get the unique object name for this object. All movable objects
		have a name that is unique within the "type" namespace (i.e. all
		entities or all lights).
	*/
	function getName():String;
	
	/**
		Get whether this movable object is attached to a scene node that
		is visible in the scene.
	*/
	function isInScene():Boolean;
	
	/**
		True if the movable object should be rendered.
	*/
	function isVisible():Boolean;
	
	/**
		Set whether this object should be rendered or not.
	*/
	function setVisible(value:Boolean);
	
	/**
		Set the visibility flags for this movable object. The
		visibility (in addition to an on/off visibility setting)
		determine the visibility of an object in the scene based
		on the scene's current visibility flag settings.
		<p>
		Visibility flags also implicitly define lighting inclusion
		and exclusion lists. In order for a movable object to be
		lit by a particular light in the scene, it must have one
		or more visibility flags set to match that of the light
		(i.e. a bit-wise AND of this movable object's flags and
		that of the light must be non-zero). A movable object that
		is not lit by any lights may still be visible in the scene
		due to ambient lighting or other material settings, but it
		will not be dynamically lit by a light source unless they
		have this visibility flag intersection.
		
		@see Scene#setVisibilityMask
	*/
	function setVisibilityFlags(flags:Integer);
	
	/**
		Get the current visibility flag settings for this movable
		object.
	*/
	function getVisibilityFlags():Integer;
	
	/**
		Set the query flags used by this movable object during a
		scene query. These are used to narrow the list of possible
		objects considered during a scene query and can greatly
		increase query performance.
	*/
	function setQueryFlags(flags:Integer);
	
	/**
		Get the current query flags for the movable object.
	*/
	function getQueryFlags():Integer;
	
	/**
		Enable or disable shadow casting for this movable object.
		If this is a light, this enables it's ability to be a
		source of shadows.
	*/
	function setCastShadows(value:Boolean);
	
	/**
		Get whether this movable object casts shadows. If it's a
		light, it means shadows will eminate from it, otherwise it
		means to actually "cast" shadows.
	*/
	function getCastShadows():Boolean;
	
	/**
		Gets the bounding sphere radius for this movable object.
	*/
	function getBoundingRadius():Float;

	/**
		Get a copy of the bounding box for this movable object.
		<p>
		Note: Bounding boxes in Ogre are not exact. They are generally
		quite close, but are primarily used only for scene optimization
		(visibility, etc) not exact collision.
	*/
	function getBoundingBox():AxisAlignedBox;
	
	/**
		Get a copy of the bounding box of this movable object in world
		coordinates.
		<p>
		Note: Bounding boxes in Ogre are not exact. They are generally
		quite close, but are primarily used only for scene optimization
		(visibility, etc) not exact collision.
	*/
	function getWorldBoundingBox():AxisAlignedBox;
	
	
	/**
		Assign this movable object to an explicit rendering queue group.
		These are hard divisions in the scene rendering order, allowing
		you to explicitly state that some objects render before or after
		others. Higher numbers are rendered after lower ones, with the
		"default" value being 50. Other notable render queue groups:
		<ul>
		<li>0 = background</li>
		<li>5 = skybox/skydome (draw first mode)</li>
		<li>25 = world geometry (terrain, etc)</li>
		<li>50 = normal/default (entities, etc)</li>
		<li>95 = skybox/skydome (draw after mode)</li>
		<li>100 = overlays</li>
		</ul>
		<p>
		Warning: abuse of this for scene ordering will adversely affect
		performance. You should generally rely on the rendering engine
		to provide the most efficient ordering mechanisms possible, and
		only use this when necessary.
		
		@param groupId A value between 1 and 105 indicating which group
			the object should be rendered with.
	*/
	function setRenderQueueGroup(groupId:Integer);
	
	/**
		The the render queue group that this movable object belongs to.
		Unless you've changed it, it will return 50 (the default).
	*/
	function getRenderQueueGroup():Integer;
	
	/**
		Get the query flags value that all new MovableObjects get
		upon creation. Default: 1
	*/
	static function getDefaultQueryFlags():Integer;
	
	/**
		Set the query flags value that all new MovableObjects will
		get when first created. Default: 1
	*/
	static function setDefaultQueryFlags(flags:Integer);
	
	/**
		Get the visibility flags value that all new MovableObjects get
		when first created. Default: 1
	*/
	static function getDefaultVisibilityFlags():Integer;

	/**
		Set the visibility flags value that all new MovableObject get
		when first created. Default: 1
	*/	
	static function setDefaultVisibilityFlags(flags:Integer);
}

/**
	An animation unit encompasses a single logical partitioning
	of animation state. Much like a texturing unit is responsible
	for spitting out textures in a GPU, this abstraction is
	responsible for driving a single animation state (or blending
	it into another).
	<p>
	By combining multiple animation units on an entity, you can
	provide independently animated sections (such as the torso
	and legs of a character) and have multiple animation states
	playing at once. You can also speed up or slow down animations
	independently using this partition.
*/
class AnimationUnit
{
	/**
		Get whether this animation unit is currently enabled.
		If not enabled, the animation will not advance during
		the normal course of rendering.
	*/
	function isEnabled():Boolean;
	
	/**
		Enable this animation unit. An animation unit must be
		enabled before it will automatically play or blend an
		animation state that it governs.
		
		@param value True to enable this animation unit.
	*/
	function setEnabled(value:Boolean);
	
	
	/**
		Set the animation state to "return to" when the current
		animation or blend step has been completed. If this
		is set to the empty string, the animation unit will
		stop playing entirely (but remain enabled).
		
		@param anim The entity animation state to use for the
			"idle" state for this unit.
	*/
	function setIdleState(anim:String);
	
	/**
		Get the current animation time scale factor.
		@see setTimeScaleFactor
	*/
	function getTimeScaleFactor():Float;
	
	/**
		Set the current animation time scale factor for this
		entity. This will speed up or slow down skeletal
		animations for the entity by multiplying the time
		scale factor by the "real" amount of time that is
		added each frame, in essence speeding or slowing the
		passage of time.
		<p>
		For example, to double how quickly an animation plays,
		pass 2.0 here. Or, to make an animation run at half
		speed, pass 0.5.
		
		@param factor The new time scale factor to use when
			animating. Must be > 0, but large values (greater
			than about 3.0 or 4.0) are not likely to provide
			good results with typical animations (depends on
			the actual animation however).
	*/
	function setTimeScaleFactor(factor:Float);

	/**
		Immediately switch to a given animation without performing
		any animation blending. This supercedes any blend operation
		already under way (if any).
		@param anim The animation state to switch to.
		@param loop If true, the unit will remain in the new state
				 rather than return to its idle state.
	*/
	function animationSwitch(anim:String, loop:Boolean);

	/**
		Blend the current animation state to the beginning of the
		specified animation state. This supercedes any existing
		animation blend operation that may be in progress.
		@param anim The animation state to blend to.
		@param duration The duration of the blend.
		@param loop If true, the unit will remain in the new state
				 rather than return to its idle state.
	*/
	function animationBlend(anim:String, duration:Float, loop:Boolean);

	/**
		Blend the current animation state to the specified animation
		state over time (i.e. use both the source and target animations
		during the blend). This supercedes any existing animation blend
		operation that may be in progress.
		@param anim The animation state to blend to.
		@param duration The duration of the blend.
		@param loop If true, the unit will remain in the new state
				 rather than return to its idle state.
	*/
	function animationCrossBlend(anim:String, duration:Float, loop:Boolean);
	
	/**
		Return true if the animation state is currently in the process
		of blending to a "target" animation.
	*/
	function isBlending():Boolean;
	
	/**
		Get the primary (source) "play head" time position in seconds.
		If not animating, this returns -1.0.
	*/
	function getTimePosition():Float;
	
	/**
		Set the primary (source) "play head" time position in seconds.
		Setting the time position during a blend operation does not
		affect the blend length or weights, it simply repositions the
		time pointer for the given play head. If not animating, this
		does nothing. 
	*/
	function setTimePosition(t:Float);
	
	/**
		Get the length of the primary (source) animation in seconds.
		If not animating, this returns 0.0.
	*/
	function getLength():Float;
	

	/**
		Get the secondary (target) "play head" time position in seconds.
		If not animating or blending, this returns -1.0.
	*/
	function getTargetTimePosition():Float;
	
	/**
		Set the secondary (target) "play head" time position in seconds.
		Setting the time position during a blend operation does not
		affect the blend length or weights, it simply repositions the
		time pointer for the given play head. If not animating or
		blending, this does nothing. 
	*/
	function setTargetTimePosition(t:Float);
	
	/**
		Get the length of the primary (source) animation in seconds.
		If not animating, this returns 0.0.
	*/
	function getTargetLength():Float;

}

/**
	An movable scene object that is based on a mesh definition. It may also
	be animated by one or more skeleton. This is actually a superset of the
	default Ogre::Entity class with some extra functionality (such as the
	ability to change a mesh at runtime and perform animation blending).
	(See Ogre::Entity and CSM::ExEntity).
*/
class Entity extends MovableObject
{
	/**
		Get the animation states defined for this entity. If the
		entity is not animated, or has no animations loaded, this
		will be an array with 0 elements. Otherwise, it contains
		the strings representing the animation state names.
	*/
	function getAnimationStates():Array;
	
	/**
		Get the number of animation units currently defined for
		this entity.
	*/
	function getAnimationUnitCount():Integer;
	
	/**
		Set the number of animation units this entity needs
		for animation. If the number of units is larger than
		the current number, new units are created as needed
		(disabled by default). If the new number of units is
		less than the current number, the list will be culled
		so that only the new number of units are available for
		the entity.
		<p>
		Upon creation, an entity has no animation units defined,
		and you must explicitly set at least one in order to
		play an animation.
		
		@param units The total number of units that this entity
			requires.
			
		@see AnimationUnit
	*/
	function setAnimationUnitCount(units:Integer);
	
	/**
		Get a specific animation unit instance. You must first
		create one or more units using
		<code>setAnimationUnitCount()</code> in order to use
		this method.
		
		@param unit The 0-based index of the unit to fetch. Must
			be &gt;= 0 and &lt; the total animation unit count.
	*/
	function getAnimationUnit(unit:Integer):AnimationUnit;
	
	/**
		Attach an object to a bone in the entity's skeleton.
		
		@param bone The name of a bone in the entity's skeleton.
		@param object The movable object to attach.
		@param orientation (Optional Quaternion) An additional orientation
				to use when attaching to the target bone.
		@param positionOffset (Optional Vector3) A positional offset to use
				when attaching to the target bone.
	*/
	function attachObjectToBone(bone:String, object:MovableObject, ...);
	
	/**
		Detach an object from its parent bone. The object must have
		been previously attached via attachObjectToBone().
	*/
	function detachObjectFromBone(object:MovableObject);
	
	/**
		Override the textures used by this entity by providing new
		ones using the texture aliasing system. This allows you to
		swap the textures used at runtime without having to previously
		allocate or derive a material explicitly.
		<p>
		For example, if the main texture unit in the material of a
		mesh has a "Diffuse" alias name,then you could use this like so:
		<pre>
			_scene.getEntity("Dude01").applyTextureAliases(
				{ Diffuse="redcoat.png" }); // Now he's british! ;)
		</pre>
		<p>
		After doing the above, the entity will have dynamically generated
		materials that are cloned from the original mesh's materials,
		but substituting new textures for those specified in the arguments
		to this method.
		<p>
		The process gets slightly more complicated for multi-material
		meshes/entities: you must specify an alias mapping for each
		sub-mesh/sub-entity. So, perhaps you've got a car model with an opaque
		shiny part (the body, submesh 0), and the transparent windows
		(submesh 1). There are two distinct materials (or at least likely
		are), thus you must supply aliases for each. Like so:
		<pre>
			carEntity.applyTextureAliases({ Diffuse="scratched_metal.png" }, 0);
			carEntity.applyTextureAliases({ Diffuse="broken_glass.png" }, 1);
		</pre>
		<p>
		You can override as little or as many textures as you want for each
		sub-entity (but the alias must exist in the material specification
		for the texture to be found and replaced). If you do not set a
		specific alias, it will retain whatever the value was in the original
		material.
		<p>
		<b>NOTE:</b> Texture alias changes are determined -by name only-. This
		means that even if an underlying texture has had its data changed (for
		instance, a procedural texture redefined), if the name hasn't changed,
		the old texture will continue to be used. To make sure that the new
		texture is used, verify the texture named is not the same as the
		current value (e.g. add a unique id to the end of the texture name
		when creating it).
		
		@param aliases A table "Alias" =&gt; "TextureName" mappings for a
		    sub-entity of this entity.
		@param subEntityIndex (Optional) The sub-entity to change aliases
			for. This defaults to 0, and works for single material objects,
			but in a multi-material entity, you will have to explicitly change
			each in order for them to take effect.
	*/
	function applyTextureAliases(aliases:Table, ...);
	
	
	/**
		Set the object-level opacity override for this entity. The opacity
		is essentially the alpha value but at the object-level, and applies
		on top of any existing alpha blending operations specified by the
		underlying materials used by the entity. This will automatically
		apply to all sub-entities and materials as needed.
		<p>
		Use this to "fade" objects in or out of a scene. Be warned, though,
		that while active (opacity &lt; 1.0f), there is a performance hit
		for performing scene blending.
		<p>
		This is set to 1 (disabled) when the entity is first created.
		
		@param opacity A value from 0 (fully transparent) to 1 (transparency
				fully determined by entity's material), inclusive.
	*/
	function setOpacity(opacity:Float);
	
	/**
		Get the current opacity value for this entity.
	*/
	function getOpacity():Float;
	
	/**
		Set the per-entity diffuse color override. If the alpha value of
		the given color is greater than 0, it will blend all of the
		diffuse colors in the original material with the new one (scaled
		by alpha). Default: Color(0,0,0,0) (i.e. none)
	*/
	function setDiffuse(color:Color);
	
	/**
		Get the current diffuse color blend for this entity.
	*/
	function getDiffuse():Color;
	
	/**
		Set the per-entity ambient color override. If the alpha value of
		the given color is greater than 0, it will blend all of the
		ambient colors in the original material with the new one (scaled
		by alpha). Default: Color(0,0,0,0) (i.e. none)
	*/
	function setAmbient(color:Color);
	
	/**
		Get the current ambient color blend for this entity.
	*/
	function getAmbient():Color;

	/**
		Discard any material instancing this entity may have created
		and reset to the materials specified by the mesh. You only
		need to call this if you explicitly want to restore the original
		condition of the entity, otherwise the instanced materials
		will be released when the entity is destroyed.
	*/
	function resetMaterials();
	
	
	/**
		Get the current position (relative to the entity) for a
		specific bone of this entity. This will throw an error if
		the entity has no skeleton or does not have the given bone.
	*/
	function getBonePosition(boneName:String):Vector3;
	
	/**
		Get the current orientation (relative to the entity) for
		a specific bone of this entity. This will throw an error
		if the entity has no skeleton or does not have the given bone.
	*/
	function getBoneOrientation(boneName:String):Vector3;
	
	
	/**
		Return an array containing any objects that have been attached
		to this entity's bone structure.
	*/
	function getAttachedObjects():Array;
}

/**
	Cameras are used to control the rendering view. They are attached to a
	viewport (which is part of a rendering window). (See Ogre::Camera)
*/
class Camera extends MovableObject
{
	static PM_POINTS = 1;
	static PM_WIREFRAME = 2;
	static PM_SOLID = 3;
	
	/**
		Set the polygon rendering mode that this camera should
		use when rendering the scene. This must be one of the
		predefined modes: PM_POINTS, PM_WIREFRAME, or PM_SOLID.
		Default: PM_SOLID.
	*/
	function setPolygonMode(mode:Integer);
	
	/**
		Get the current rendering mode for polygons. This
		returns one of PM_POINTS, PM_WIREFRAME, or PM_SOLID.
	*/
	function getPolygonMode():Integer;
	
	/**
		Get the far clipping plane distance.
	*/
	function getFarClipDistance():Float;
	/**
		Set the far clipping plane distance.
	*/
	function setFarClipDistance(value:Float);
	
	/**
		Get the near clipping plane distance.
	*/
	function getNearClipDistance():Float;
	/**
		Set the near clipping plane distance.
	*/
	function setNearClipDistance(value:Float);
	
	/**
		Get the camera's current vertical field-of-view in radians.
	*/
	function getFOVy():Float;
	/**
		Set the camera's vertical field-of-view. FOV values generally determine
		the type of "lens" being used to render. High values (90+ degrees) result
		in a "fish eye" style wide angle view, whereas lower values (30- degrees)
		result in a more telescopic "zoomed in" style view. Typical values are
		between 45 and 60 degrees. (Note, even though degrees are mentioned here,
		it's purely for human/documentation reasons. The paramter takes
		<i>radians</i>.) Default: 0.16667 (~30 degrees)
		
		@param fov New field-of-view in radians.
	*/
	function setFOVy(fovy:Float);

	/**
       Set and enable/disable the camera's custom projection matrix.
		This is somewhat advanced and in most cases is not what you
		want to do, but allows you to do some things like set up a
		scaled isometric projection matrix (as used by the minimap).
        <pre>
         _                                   _
        |  mat_00   mat_01   mat_02   mat_03  |
        |  mat_10   mat_11   mat_12   mat_13  |
        |  mat_20   mat_21   mat_22   mat_23  |
        |_ mat_30   mat_31   mat_32   mat_33 _|
        </pre>

        @param useCustom Whether to use a custom projection matrix
                           (as opposed to the ogre-generated one)

        @param mat_ij The value to put in the projection matrix at position mat[i][j]
	*/
	function setCustomProjectionMatrix(useCustom:bool,
                                       mat_00:float, mat_01:float, mat_02:float, mat_03:float,
                                       mat_10:float, mat_11:float, mat_12:float, mat_13:float,
                                       mat_20:float, mat_21:float, mat_22:float, mat_23:float,
                                       mat_30:float, mat_31:float, mat_32:float, mat_33:float);

}

/**
	A light emitter in the scene. This class encompasses all types
	of movable light sources within the scene. (See Ogre::Light)
*/
class Light extends MovableObject
{
	static POINT = 0;
	static DIRECTIONAL = 1;
	static SPOTLIGHT = 2;
	
	/**
		Set the type of light that this light represents. This
		must be one of POINT, DIRECTIONAL, or SPOTLIGHT. All
		lights are POINT lights by default.
	*/
	function setLightType(lightType);

	/**
		Set the light attenuation parameters. These affect the
		lighting calculations and are the primary way of changing
		how a light behaves with respect to the lighting system
		(other than color). In a nutshell, the luminosity (or
		relative intensity of a light) is defined as: 1 / <i>Attenuation</i>.
		Attenuation models how quickly the light disperses into the
		environment as the distance from the light increases. It 
		uses a formula that looks like this:
		<pre>
			Attenuation = <i>Constant</i> + <i>Linear</i> * d + <i>Quadratic</i> * d^2
		</pre>
		It is these three coefficients that you're setting by calling
		this function.
		<p>
		The first parameter is not used in lighting calculations other
		than trivial exclusion tests. I.e. any object further than <tt>range</tt>
		away will not be considered lit by this light <i>regardless of the
		attenuation values</i>. Thus, if the range value is "smaller" than
		your desired maximum attenuation range, objects will "snap" into
		lightness as they come into range of the light. Generally you want
		to keep this in sync with the theoretical range of a light based
		on its attenuation. 
		<p>
		Because dealing with attenuation parameters can be somewhat
		unintuitive. Here is a table of some typical ranges and what
		attenuation values can be used. Note, these ranges assume
		"black" is anything darker than about 0.05 intensity. If you
		require a different definition of black, you will need to 
		perform some calculation tests of your own. These all use
		quadratic attenuation exclusively, you could provide different
		curves by using linear (or a combination of both).
		<table>
		<tr><th>Range</th><th>Constant</th><th>Linear</th><th>Quadratic</th></tr>
		<tr><td> 800  </td><td> 1.0 </td><td> 0 </td><td> 0.00003 </td></tr>
		<tr><td> 350  </td><td> 1.0 </td><td> 0 </td><td> 0.00016 </td></tr>
		<tr><td> 150  </td><td> 1.0 </td><td> 0 </td><td> 0.00085 </td></tr>
		<tr><td> 75   </td><td> 1.0 </td><td> 0 </td><td> 0.0035 </td></tr>
		<tr><td> 25   </td><td> 1.0 </td><td> 0 </td><td> 0.1 </td></tr>
		</table>
		
		@param range This is the maximum range for the light in
			world units. Default: 100000
		@param constant This affects how much attenuation is
			performed. It ranges from 0.0 (complete attenuation)
			to 1.0 (no attenuation). Default: 1.0
		@param linear The linear component in lighting calculations.
			In this, 1.0 means attenuate evenly across the distance.
			Default: 0.0
		@param quadratic The quadratic component in lighting calculations.
			This affects the "curvature" of the attenuation function.
			Default: 0.0
	*/	
	function setAttenuation(range:Float,
			constant:Float, linear:Float, quadratic:Float);


	/**
		Get the diffuse (i.e. general) color of light that this light
		source exudes into the scene.
	*/
	function getDiffuseColor():Color;
	
	/**
		Set the diffuse color that this light will exude into the scene.
		The default is white (1,1,1,1).
	*/
	function setDiffuseColor(color:Color);
	
}


/**
	A particle system encompasses a set of emitters and affectors
	as well as all the particles in the system. It provides a
	single "world origin" for the particle system and various
	methods for controlling it.
*/
class ParticleSystem extends MovableObject
{
	/**
		Get the current number of particles in the system.
	*/
	function getNumParticles():Integer;
	
	/**
		Erase all particles from the system.
	*/
	function clear();
	
	/**
		Get the maximumn number of particles this system can
		have at once.
	*/
	function getParticleQuota():Integer;
	
	/**
		Set the maximum number of particles that this system
		can have active at once.
		
		@param quota Maximum active particles for this system.
	*/
	function setParticleQuota(quota:Integer);
	
	/**
		Advance the system forward the given number of seconds
		using the specified time interval. This "warps" the
		particle system forward in time without actually having
		to wait for it the time. This is useful to jump start
		particle systems and avoid a lead in time.
		
		@param time The total number of seconds to advance.
		@param interval The interval to use when advancing 
			<code>time</code> seconds.
	*/
	function fastForward(time:Float, interval:Float);
	
	/**
		Set the time scale factor for this particle system. This
		value is multiplied by the amount of actual time passed
		during each frame rendering, so you can effectively
		speed up or slow down the passage of time by setting
		this to something other than 1.0. For instance,
		0.5 will make the system run half as fast, whereas
		2.0 will make it run two times the normal speed.
		<p>
		The default for all newly created particle systems is
		1.0.
		
		@param value The new time scale factor.
	*/
	function setTimeScaleFactor(value:Float);
	
	/**
		Get the current time scale factor for this particle
		system.
		
		@see #setTimeScaleFactor()
	*/
	function getTimeScaleFactor():Float;
	
	/**
		Set the timeout before this particle system will 
		stop updating when not visible. To avoid using spare
		processor cycles, a particle system should stop simulating
		when non-visible. This timeout will automatically disable
		the system after the specified number of seconds 
		have elapsed and it has still not become visible.
		<p>
		Default: all particle systems will render continuously
		(timeout = 0).
		
		@param timeout Number of seconds before the particle system
			disables itself when not visible. To disable the
			non-visible timeout functionality, pass 0.
	*/
	function setNonVisibleUpdateTimeout(timeout:Float);
	
	/**
		Get the number of seconds before a particle system
		stops updating itself when non-visible.
	*/
	function getNonVisibleUpdateTimeout():Float;
	
	/**
		Set the expected (or initial) bounds for this particle
		system. This is used to determine visibility. If you
		would like to have this calculated every frame, try the
		setBoundsAutoUpdated().
		
		@param bounds The bounding box
		
		@see #setBoundsAutoUpdated()
	*/
	function setBounds(bounds:AxisAlignedBox);
	
	/**
		Set whether the movable object bounding box will be
		automatically calculated (possibly for a limited
		amount of time). Generally particles have a finite
		lifetime so the bounding box for the entire system
		winds up stabilizing after a finite amount of time.
		Thus, it is best to avoid recalculating the bounding
		box every frame, but this method allows you to do so.
		<p>
		If set to auto-calculate the bounding box, an optional
		timeout value can be specified which will disable this
		calculation after a certain amount of time (presumably
		once the particle system has stabilized).
		<p>
		The default for new systems is to auto calculate the
		bounding box for 5 seconds and then keep the last
		result.
		
		@param autoUpdate If true, the bounding box will be
			updated every frame (or at least until the timeout
			occurs, if present). Otherwise the bounding box
			will <b>not</b> be calculated and you must set it
			using setBounds().
		
		@param stopIn (Optional) This is the number of seconds
			to wait before disabling the auto-update feature.
			This only applies if the first parameter is true.
	*/
	function setBoundsAutoUpdated(autoUpdate:Boolean, ...);
	
	
	/**
		Remove all emitters from this particle system. This is primarily
		useful only in gracefully fading a particle system away. This will
		stop any more particles from being emitted, but existing particles
		will continue on their normal lifespan until dead. If you're waiting
		for it to "die", you can periodically check {@link #getNumParticles()}.
	*/
	function removeAllEmitters();
}


/**
	Grid layers contain the actual data information that is stored in a grid.
	Currently they store only integer data values. You should not be creating
	layers directly. Instead, use the Grid class to instantiate and manage
	layers for a specific grid.
*/
// Phase III
/*
class GridLayer
{
	function getGrid():Grid;
	function getIndex():Integer;
	function getName():String;
	function isVisible():Boolean;
	function setVisible(value:Boolean);
}
*/

/**
	Grids contain one or more 2D arrays of data values (on "layers"). The
	grid can contain multiple layers of data representing different things.
	Currently, only integer data values are supported, but the interpretation
	of that data is left to the user. For instance, it could represent a
	height field or collision information, or perhaps indexes into a texture
	palette for use in a terrain situation.
	<p>
	A grid has a specific 2D dimension, which represents the number of grid
	data points in each of its 2 axes. This is basically the same as the
	resolution of an image. For instance, a 20x20 grid contains 400 grid
	points.
	<p>
	Grids can use their position, orientation, and size information in the
	scene to translate 3d points into 2d grid points. This is obviously useful
	to quickly lookup data values in the grid.
*/
// Phase III
/*
class Grid
{
	function getWidth():Integer;
	function getHeight():Integer;
	function setDimensions(w:Integer, h:Integer);

//	function get2dCoord(pos:Vector3):Vector2;
	
	function addLayer(name:String);
	function getLayer(index:Integer);
	function getLayerByName(name:String);
	function removeLayer(index:Integer);
}
*/

/**
	Sound nodes contain a 3d position (and orientation) used to play 3d
	positional sounds. It wraps a lower-level non-positioned sound and
	provides the extra information needed to "render" the sound in the
	environment.
*/
class SoundEmitter extends MovableObject
{
	/**
		Set the sound that this emitter should use when playing. This
		is a name of a sound file/resource (e.g. "windchimes1.ogg").
		Usually this is set in the Scene.createSoundEmitter(), but this
		allows the sound that's played to be changed at a later time.
		<p>
		If the emitter was playing before calling this method, it
		will continue playing, but from the start of the new sound
		buffer.
		
		@param sound The sound file/resource to use when playing.
			You may also use "" here to specify that it should release
			any current buffer and not load another (not generally
			needed since buffers are released automatically as
			needed using normal resource loading mechanisms).
	*/
	function setSound(sound:String);
	
	/**
		Get the sound currently set to be played by this emitter. Note
		that the sound may not actually BE playing currently.
	*/
	function getSound():String;
	
	/**
		True if the sound is currently being played.
	*/
	function isPlaying():Boolean;
	
	/**
		True if the sound is set to play, but is currently paused.
	*/
	function isPaused():Boolean;
	
	/**
		Set the sound to play if it is not already set to do so. If
		the sound is looping, it will continue to play until explicitly
		told to stop, otherwise it will stop when sound resource is
		completely played. If not looping and the end is reached, the
		play head will automatically be set to the start of the buffer
		so it can be played again.
	*/
	function play();
	
	/**
		Pause current playback on this emitter without moving the virtual
		play head in the sound buffer. Contrast this with stop() which
		will reset the play head when called.
	*/
	function pause();
	
	/**
		Stop playback of this sound if it is currently playing. Stopping
		a sound implicitly resets the the play head to the beginning of
		the sound buffer. (Thus, a subsequent call to play() will start
		at the beginning of the sound.) Use pause() to stop sounds while
		retaining the play head position.
	*/
	function stop();
	
	/**
		Is this emitter currently set to loop the specified sound?
	*/
	function isLooping():Boolean;
	
	/**
		Set the looping flag for this sound emitter. When looping, the
		emitter will automatically rewind the play head and begin playing
		at the start (with no sound gap).
	*/
	function setLooping(value:Boolean);

	/**
		Set the gain value for this sound. Gain is a relative increase or
		decrease in volume for the specific sound to use when mixing.
	*/
	function setGain(gain:Integer);
	
	/**
		Get the current gain mixing value for this sound.
	*/
	function getGain():Integer;

	/**
		Set the priority of the sound to use when mixing. Mixing can be
		constrained to a sometimes small subset of active sounds (e.g.
		sounds cards with only 6 channels for mixing). Priorities allow
		the system to select more important sounds over less important
		ones (for instance, the "attack" sound would likely have a
		higher priority than an ambient bird chirping sound).

		@param priority
				The new priority value for this sound.
				<p>
				The default priority is 0, and larger values are
				of higher priority than lower values. Negative
				priorities are fine.
	*/
	function setPriority(value:Integer);

	/**
		Get the current mixing priority level for this sound.
	*/
    function getPriority();
}


/**
	A decal is a simple polygon with a textured surface that can be moved
	around. They can be expensive on some hardware so don't go overboard with 
	a scene. Consider limiting them to a fixed number and reusing them via
	an age-based eviction policy.
*/
class Decal extends MovableObject
{
	/**
		Get the normal for the decal polygon. This is facing "out" of
		the polygon surface.
	*/
	function getNormal():Vector3;
	
	/**
		Set the polygon surface normal. The default normal is <tt>Vector3.UNIT_Y</tt>.
	*/
	function setNormal(normal:Vector3);
	
	/**
		Set a local positional offset for this decal (relative to its
		parent scene node). This position is the center of the polygon
		(as opposed to a top/left type of reference).
	*/
	function getPosition():Vector3;
	
	/**
		Set the center position for the decal. The decal vertices will be
		extruded perpendicular to the normal using this central position.
		The default position is (0,0,0).
	*/
	function setPosition(position:Vector3);
	
	/**
		Set the total planar area covered by this decal. This determines
		how far away from the center point the vertices are (effectively
		determining how "big" the decal is).
	*/
	function setSize(width:Float, height:Float);
	
	/**
		Set the material for this decal (defines what texture to render)
		using the material's name. Currently, decals use the entire
		texture provided (in the future, we will likely be able to
		specify the corner UVs explicitly).
		<p>
		Note: Decals almost always need a <tt>depth_bias</tt> specified
		in their materials or you will get a flickering artifact due to
		coplanar polygons fighting for the depth buffer.
	*/
	function setMaterialName(material:String);
	
	/**
		Get the name of the material used by this decal.
	*/
	function getMaterialName():String;
}


/**
	A textboard is a simple billboard type 3D entity that displays a
	snippet of text that is always aligned to the camera. This differs
	from a 2D text element (such as those supported by Widget) in that
	it is rendered along with the rest of the scene and takes part in 
	all transforms, node animations, etc. The biggest benefit to using
	textboards over a 2D solution is that it can be clipped according
	to depth/distance.
*/
class TextBoard extends MovableObject
{
	/**
		Get the name of the font that the text board uses to render
		the text. By default this is the builtin font "BlueHighway".
	*/
	function getFontName():String;
	
	/**
		Set font used by this text board. This specifies a font that
		has been defined using the Ogre fontdef system.
	*/
	function setFontName(fontName:Vector3);
	
	/**
		Set a local positional offset in the Y-axis for this board.
		This extra offset is applied after all other transforms and
		is useful for avoiding the full overhead of an extra scene
		node in order to place the text "over" something (e.g. an
		entity's head).
	*/
	function setYOffset(offset:Float);
	
	/**
		Set the height of a single line of text in world units. This
		controls how "big" the rendered font is. If the text this
		board is displaying has multiple lines (separated by '\n'),
		the actual height will be a multiple of this value.
	*/
	function setLineHeight(height:Float);
	
	/**
		Set the text displayed by this text board. Text boards can
		display multiple lines of text (separated by '\n'), but will
		grow "downward" (negative Y axis direction) unless the text
		alignment has been explicitly set to do otherwise.
		<p>
		Currently only ASCII text is supported.
	*/
	function setText(text:String);
	
	/**
		Get the text currently displayed by this text board.
	*/
	function getText():String;
	
	/**
		Set the top color used when rendering lines of text. Each line
		of text is capable of having its top and bottom diffuse
		colors set independently. The default is pure white: (1,1,1,1).
	*/
	function setColorTop(color:Color);
	
	/**
		Get the top color used when rendering lines of text.
	*/
	function getColorTop():Color;
	
	/**
		Set the bottom color used when rendering lines of text. Each line
		of text is capable of having its top and bottom diffuse
		colors set independently. The default is pure white: (1,1,1,1).
	*/
	function setColorBottom(color:Color);
	
	/**
		Get the bottom color used when rendering lines of text.
	*/
	function getColorBottom():Color;

	/**
		Set text alignment values that will control how lines of
		text are displayed relative to the object's origin. Use
		these parameters to make text render "upward" or to left/right
		justify lines of text.
		<p>
		For each of the horizontal and vertical dimensions, a
		scalar value between 0.0 and 1.0 represents how "far"
		along each axis the line of text should be placed. For
		horizontal alignment, this controls the left (0.0) or
		right (1.0) justification. The vertical alignment is
		similar but operates on all of the lines of text for
		purposes of alignment. That is, for 0.0, all lines will
		extend "downward" from the local origin with the first line
		touching the origin point, and for 1.0, all lines are
		"above" the point, with the bottom of the last line
		touching the local origin. Each alignment value can also
		have any value within this range (such as 0.5 to center
		the text).
		<p>
		The default for newly created text boards is 0.5 and
		0.5 (i.e. middle-center the text).
	*/
	function setTextAlignment(halign:Float, valign:Float):String;
}

/**
	A texture projector works much like a slide projector (kind of like
	the opposite a camera) by projecting a single texture onto renderable
	objects within the scene. You can select only subsets of geometry
	using a query mask (for instance, only flat surfaces or terrain,
	assuming your flags have been set accordingly).
	<p>
	Be warned though! Projecting textures is expensive to do. It's
	unlikely that older cards will be able to handle more than a handful on
	the screen at any one time (because it involves rendering the
	geometry multiple times, once for each projection that touches it).
	By that same token, this technique may not apply very well to very
	dense/complex meshes due to the massive overdrawing required to
	texture them.
*/
class TextureProjector
{
	/**
		Get the name of the texture that this projector will project
		into the scene.
	*/
	function getTextureName():String;
	
	/**
		Set the name of the texture that this projector will project
		into the scene.
		@param name Name of an existing texture to use.
	*/
	function setTextureName(name:String);
	
	/**
		Get the query mask used to select objects to project onto.
	*/
	function getProjectionQueryMask():Integer;
	
	/**
		Set the query mask used to select objects to project onto.
		Note, the default when the projector is created is to query
		for all objects (mask = 0xffffffff), however, you are almost
		assuredly going to want to set this to something more limiting
		for performance reasons.
	*/
	function setProjectionQueryMask(mask:Integer);
	
	/**
		Is the frustum debug geometry currently being drawn?
	*/
	function isDrawingFrustum():Boolean;
	
	/**
		Enable/disable the drawing of a debug geometry that shows
		the projector's frustum (i.e. the "outline" of the projector
		and where is casts its "light"). This is useful when tracking
		down why you cannot see a particular texture on a surface.
	*/
	function setDrawingFrustum(enabled:Boolean);
	
	/**
		Is the projector using an "additive" blending model when
		blending the texture onto its surfaces? If this is false,
		the is modulative instead.
	*/
	function isAdditive():Boolean;
	
	/**
		Set the projector to use an additive blending mode (as opposed
		to modulative) when projecting the texture. This affects how the
		"source" texture blends with the underlying geometry. Additive
		is generally used for things that simulate actual light sources
		such as a movie theatre projector, spotlight, etc. Modulative is
		often used to "darken" an image to simulate something like shadows.
		<p>
		When initially created, the projector is modulative mode.  
	*/
	function setAdditive(value:Boolean);
	
	/**
		Is the projector using an orthographic projection? If false, this
		implies a perspective projection. Use setPerspective() and
		setOrthoWindow() to change modes (they are mutually exclusive).
	*/
	function isOrtho():Boolean;

	/**
		Set the projector to use an orthographic projection with the given
		world coordinate "window" size. Note, this also defines the aspect
		ratio of the projection (updated internally).
		
		@param width The "width" of the projection, in world units.
		@param height The "height" of the projection, in world units.
	*/
	function setOrthoWindow(width:Float, height:Float);
	
	/**
		Get the furthest distance that objects will be projected onto.
	*/
	function getFarClipDistance():Float;
	
	/**
		Set the furthest distance that objects will be projected onto.
	*/
	function setFarClipDistance(far:Float);

	/**
		Get the distance at which objects will begin to be projected onto.
	*/
	function getNearClipDistance():Float;

	/**
		Get the distance at which objects will begin to be projected onto.
	*/
	function setNearClipDistance(near:Float):Float;
	
	/**
		Force the list of objects that are receiving the projection to
		be updated. Because the scene query required to obtain the
		list of objects to be re-rendered is expensive, the projector
		caches the result and reuses it whenever possible. However,
		it is incapable of knowing when new objects may have entered
		its "visible area" so you must explicitly tell it to recreate
		its list of objects in this case. 
		<p>
		This is only necessary if you have objects moving in and out
		of a projector's region and need it to update its list. This
		is not needed when moving the projector itself (either directly
		or indirectly) as it automatically refreshes the list in this
		case. 
	*/
	function dirtyReceiverList();
}


/**
	RibbonTrails are helper objects that create "transient" geometry
	that will track one or more nodes, optionally changing some
	parameters (such as color or width) over time. Use this to 
	create sword swooshes or other dynamic looking effects. Note, that
	even though this object must be attached to a SceneNode (like
	any other MovableObject), the position of that scene node is not
	actually used by the ribbon geometry (they use the nodes that
	they are tracking to obtain their worldspace positions). 
*/
class RibbonTrail
{
		
}



/**
	A paged geometry instance allows a "lightweight" description of a
	scene to be partially instantiated on the fly at runtime using the
	position of the camera and various view distance settings.
	<p>
	This system also permits some scene optimization by creating or using
	approximations for more complicated meshes and geometry at various
	distances. This is a somewhat automatic level of detail system, but
	does not actually involve creating LOD meshes, it uses other tricks
	such as billboard impostoring and static mesh batching to reduce the
	batch count and/or complexity of the scene.
	<p>
	Generally this is used to efficiently model things like grasses and
	dense forests because creating an entity for each desired instance
	in the scene is much too time and resource intensive once the scene
	gets large enough.
	<p>
	Create, fetch, and delete instances of this class using
	Scene.createPagedGeometry() (and friends).
*/
class PagedGeometry
{
	/**
		Get the name given to this instance when it was created.
	*/
	function getName():String;
	
	/**
		Destroy this paged geometry instance and all of the associated
		scene objects, nodes, and any other geometry associated with it.
		If you previously had obtained instances to things created by
		this paged geometry instance, they are all invalidated by calling
		this method.
	*/
	function destroy();
}

/**
	Paged grasses allow simple quad-based grass meshes to be created
	on the fly using a pseudo random generation scheme and optionally
	a guiding density map.
*/
class PagedGrass extends PagedGeometry
{
	/**
		Add a layer to this paged grass geometry loader. A layer
		is capable of creating a single type of grass using various
		customization parameters. Those parameters are passed in a
		single table that has the following elements:
		<dl>
		<dt>material</dt>
		<dd>The material that generated grass mesh(es) will use.</dd>
		<dt>density</dt>
		<dd>A uniform density for grasses created using this layer.
			This is combined with the loader's overall density factor
			to determine the actual density. If a <tt>densityMap</tt>
			is specified, this option is ignored.</dd>
		<dt>densityMap</dt>
		<dd>The name of a texture to use as a mapped density value.
			The contents of the texture are used as lookup values for
			a single "monochrome" channel of density. Generally this
			will be a greyscale image indicating density, but if combined
			with the <tt>densityMapChannel</tt> option, a specific channel
			of a color image can be used (thereby either allowing a reuse
			of the texture for multiple densities or combining a density
			and color map into a single texture).</dd>
		<dt>densityMapChannel</dt>
		<dd>An optional selector for the channel of the <tt>densityMap</tt>
			to use to obtain density values. Valid values are:<br/>
			<ul>
			<li>"red" &mdash; The red channel (bits)</li>
			<li>"blue" &mdash; The blue channel (bits)</li>
			<li>"green" &mdash; The green channel (bits)</li>
			<li>"alpha" &mdash; The alpha channel (bits)</li>
			<li>"color" &mdash; The red, green, and blue channel bits are averaged
				to obtain a greyscale value that will then be used as the density</li>
			</ul>
			The default if this is omitted is "color".
		</dd>
		<dt>colorMap</dt>
		<dd>The name of a texture to use as an extra diffuse colorizing
		    map. The RGB component is used to modify the vertex colors
		    of the generated grasses.</dd>
		<dt>technique</dt>
		<dd>"quad" (a single randomly rotated quad), "crossquad" (two quads
				in an "X" configuration, randomly rotated). The default if
				omitted is "crossquad".</dd>
		<dt>fade</dt>
		<dd>"alpha" (fade using alpha transparency), "grow" (gradually increase
			the +Y height), "alphagrow" (use both alpha and grow techniques).
			The default if omitted is "alpha". Note, this only works if the
			paged geometry was defined to include a region of blending,
			otherwise it is ignored.</dd>
		<dt>animate</dt>
		<dd>An array of three floats representing the following values:<br/>
			<ul>
			<li>magnitude &mdash; Float, The "amount" to sway grasses in world units</li>
		    <li>speed &mdash; Float, The number of "sways-per-second"</li>
		    <li>disturbance &mdash; Float, A value from 0.0 (all grass synchronized)
		    	to 1.0 (chaotic) indicating the relative synchronicity of the
		     	animated grasses. I.e. a permutation on phase shift.</li>
		    </ul>
		<dt>size</dt>
		<dt>An array of 4 floats specifying the size ranges of each generated
			quad. I.e.: [ minWidth, minHeight, maxWidth, maxHeight ] For each,
			the "width" refers to the horizontal size of a quad(s) (in world units)
			and "height" refers to the +Y size in world units. </dt>
		</dl>
		<p>
		Minimally, you will probably want to specify "material" and "size".
	*/
	function addLayer(config:Table);
}

/**
	A paged geometry loader capable of instantiating CSM references on
	the fly.
*/
class PagedCSM extends PagedGeometry
{
	/**
		Add a CSM component reference to this page. The reference will
		be dynamically instantiated at the given position and using a
		provided rotation and scale.
		
		@param name The CSM component to instantiate.
		@param position The world coordinates to instantiate the scene
			component at. If the Y value is equal to -1, the scene
			manager will be used to automatically obtain the terrain
			height at the given location.
		@param yaw (Optional) Either a float (radians) holding the
			+Y axis rotation value, or a Quaternion, in which case, it
			is interpreted as an angle/axis pair, where the axis is
			assumed to be +Y and the angle taken for yaw.
		@param scale (Optional) Either a float holding a uniform
			scale factor, or a Vector3 whose x value holds the 
			uniform scale factor.
			
		@return An integer handle that can later be used to remove
			this specific reference from the page.
	*/
	function addComponent(name:String, position:Vector3, ...):Integer;
	
	/**
		Remove a previously added component reference.
		
		@param handle The handle previously obtained from addComponent().
	*/
	function removeComponent(handle:Integer);

	/**
		Remove all components previously added to this paged geometry.
	*/
	function clear();
	
	/**
		Set a mask that will be used to -prevent- entities that are
		created via CSM instantiation from partaking in normal page
		geometry batching (and simply be passed through as normal
		entities). The provided mask will be combined with the entity's
		query flags, and if non-zero, the entity will be preserved.
		<p>
		This can be useful to retain specific entities for things like
		collision geometry, rather than having them batched into a
		single mesh.
		<p>
		The default value is 0, meaning all entities will be batched.
	
		@param mask A query flag mask indicating entities should be
			passed through as entities, rather than batched geometry.
	*/
	function setEntityPreservationMask(mask:Integer);
}

/**
	UI elements are abstract components that are capable of receiving
	user input events. Anything that gets a key press or mouse movement
	event derives from this (notably Composition and Widget).
*/
class UI_Element extends MessageBroadcaster
{
	static events =
	{
		/**
			Triggered when a key is pressed (and no other UI element has the
			keyboard focus).
			@param evt The key event.
		*/
		onKeyPressed = function(evt:KeyEvent){}
		/**
			Triggered when a key is released (and no other UI element has
			the keyboard focus).
			@param evt The key event.
		*/
		onKeyReleased = function(evt:KeyEvent){}
		
		/**
			Triggered when a key or sequence of keys generate a typeable
			character. This is a higher level event than key up or down. It's
			built out of inspecting key states and key state changes and
			intuiting the proper result from it. For instance, the sequence
			of keys: KC_SHIFT, KC_A will result in "A" being returned here.
			Keys do not have to necessarily be released in order to generate
			a typed event.
			<p>
			This has no impact on the normal up and down events. They are
			still triggered as normal. However, this may be generated as
			well if applicable.
			<p>
			The character is a valid character according the current text
			encoding (ASCII/Unicode).
		*/
		// Ph.III (maybe)
//		onKeyTyped  = function(keyChar:String){}


		/**
			Triggered when a mouse button is pressed. UI elements can cause
			the composition to not receive this event.

			@param evt The mouse event.
		*/
		onMousePressed = function(evt:MouseEvent) {}
		
		/**
			Triggered when a mouse button is released. UI elements can cause
			the composition to not receive this event.

			@param evt The mouse event.
		*/
		onMouseReleased   = function(evt:MouseEvent) {}
		
		/**
			Triggered when the mouse is moved in the client area. UI elements
			can cause the composition to not recieve this event.
			
			@param evt The mouse event
		*/
		onMouseMoved = function(evt:MouseEvent) {}

		/**
			Triggered when the mouse enters the extents of the widget.
		*/		
		onMouseEnter = function(evt:MouseEvent) {}
		
		/**
			Triggered when the mouse exits the extents of the widget.
		*/
		onMouseExit = function(evt:MouseEvent) {}

		/**
			Triggered when the user scrolls the (vertical) mouse
			wheel. Use the <code>units_v</code> field of the provided
			event to see how much (and what direction) the wheel
			has been scrolled.
			<p>
			The units_v will contain a positive or negative scalar
			value representing the number of "clicks" the wheel
			has been scrolled. Each unit here, represents an "item"
			or "line" that should be scrolled (if applicable).
		*/
		onMouseWheel = function(evt:MouseEvent) {}
	};

	/**
		Grab keyboard focus from any element that may have it. A UI element that
		has keyboard focus will receive all keyboard events until focus is moved
		to a different element. If the element is destroyed, focus returns to the
		root composition.
	*/
	function requestKeyboardFocus();

	/**
		Does this element have keyboard focus?
	*/
	function hasKeyboardFocus():Boolean;
}

/**
	A composition encompasses all the scene(s), data files, available logic
	scripts, and other types of information that can be used in the player.
*/
class Composition extends UI_Element
{
	/**
		Create a new procedural texture definition. This will create
		a new procedural texture definition in the texture blending
		engine. Once created, this texture can be referenced in any
		place that a normal texture could be referenced (for instance
		in a material or in a derived mesh instance).
		<p>
		
		The two variants of the last arguments to this function are:
		<ul>
		<li>width:Integer, height:Integer, color:ColourValue, [mips:Integer] --
				Creates a texture of the given size with a solid background.
				If the optional mips parameter is specified, it controls the
				number of mipmaps that will be generated (default: 0).</li>
		<li>srcTextureName:String, [mips:Integer] --
				Create a texture of the same size as the input texture,
				and using the texture's contents as a background. If the
				optional mips parameter is specified, specifies the number
				of mipmaps to generate (default: 0).</li>
		<ul>
		
		@param textureName The name of the newly created texture. Must
					be unique.
	*/
	function createProceduralTexture(textureName, ...);
	
	/**
		Forces the mini-map to redraw its background
	*/
	function updateMiniMapBackground( );

	/**
		Set the visibility mask to use for the MiniMap camera.
		
		@param value The integer visibility mask value to use. This should
			     one or more of the VisibilityFlags constants.
	*/
	function setMiniMapVisibilityMask(value:Integer);

	/**
		Set the name of the texture to use for minimap stickers. MiniMap
		stickers are small quads that are drawn on the map to represent
		the location of creatures.
		
		@param name The name of the texture to use (i.e. 'DefaultSkin/Sticker.png')
	*/
	function setMiniMapStickerTexture(name:String);	

	/**
		Set the width of the minimap view area in pixels.
		
		@param value The width of the view area of the minimap, in pixels.
	*/
	function setMiniMapZoom(value:Integer);


	/**
		Set the center point of the minimap view area. This should be updated each frame
		with the new position of the avatar. The minimap background will only be redrawn
		when the player moves a specific distance away from the previously set center.
		
		@param x The X coordinate of the new center, in world units
		@param Z The Z coordinate of the new center, in world units
	*/
	function setMiniMapZoom(x:Float, z:Float);

	/**
		Create a cloned version of a mesh by changing the textures
		used (via aliasing). This allows us to clone a mesh, even
		the materials used, while supplying overrides for textures
		in those new materials. For example, if the main texture
		unit in the material of a mesh has a "Diffuse" alias name,
		then you could use this like so:
		<pre>
			_root.createDerivedMesh(
				"AmericanDude.mesh",
				"BritishDude.mesh",
				{ Diffuse="redcoat.png" });
		</pre>
		<p>
		After doing the above, the mesh "BritishDude.mesh" will exist
		just as if it were loaded from disk. Thus, you can create
		entities that reference the new mesh.
		<p>
		The process gets slightly more complicated for multi-material
		meshes: you must specify an alias mapping for each sub-mesh
		in the mesh. So, perhaps you've got a car model with an opaque
		shiny part (the body, submesh 0), and the transparent windows
		(submesh 1). There are two distinct materials (or at least likely
		are), thus you must supply aliases for each. Like so:
		<pre>
			_root.createDerivedMesh(
				"Car.mesh",
				"Car/Broken", // Note: doesn't end in .mesh
				{ Diffuse="scratched_metal.png" },
				{ Diffuse="broken_glass.png" });
		</pre>
		<p>
		Note, you can override as little or as many textures as you want
		for each sub-mesh. If you do not set a specific alias, it will
		retain whatever the value was in the original mesh.
		
		@param srcMesh The original mesh you want to clone.
		@param meshName The name of the new mesh (this doesn't have to end
			in .mesh). It must be unique (i.e. the mesh cannot already exist).
		@param aliases... A table of strings for each sub-mesh in the original
			mesh.
			
		@throws If the source mesh doesn't exist.
		@throws If the destination mesh already exists.
		@throws If the number of alias tables do not match the sub-mesh count.
		@throws If any of the alias tables are not actually tables.
	*/
	function createDerivedMesh(srcMesh:String, meshName:String, ...);
	
	/**
		Enabled/disables level of detail calculations for the terrain
	*/
	function setTerrainLODEnabled(which:Bool);

	/**
		Forces the terrain material to use a specific technique name
		instead of the default. Useful for editors where you want to
		render some special visualization geometry (Like polygon outlines
		or height information).

		@param technique The name of the technique to use. Pass a blank string to use the default 
				 technique of the terrain material.
	*/
	function setTerrainTechniqueOverride(technique:String);
	
	/**
		Create (or reuse) a material by applying the given texture aliases
		to it. The material will be created by taking the original material
		and replacing each alias with those given in the aliases table
		(which must be a simple name=value pair style table).
		<p>
		This function returns the name of the new (derived) material which
		can then be set or used elsewhere.
		<p>
		The new material (and in fact the old one too) will be collected
		along with the normal Ogre resource system, so there is no need
		to explicitly destroy the created material.
	*/
	function createMaterialUsingAliases(materialName, aliases):String;
	
	/**
		Set or update the texture(s) used by all passes/techniques that
		material has defined. You can use this to dynamically change the
		texture a material uses on the fly at runtime. The material must
		have the proper texture aliases already defined within its
		definition however.
		@param materialName The name of the material to modify.
		@param textureAliases A table mapping alias "name" (String) to 
			texture name (String) to use.
		@returns True if the matieral was actually modified (one or
		 more aliases were found and updated), or false if not.
	*/
	function setMaterialTextures(materialName, textureAliases):Boolean;
	
	
	/**
		Inspect a material and fetch the current (aliased) textures 
		found in any technique or pass in the material. Note, this will
		attempt to enumerate all texture units that have an alias name
		set regardless of what technique or pass it's in. It will save
		only the first value encountered, so if there are multiple passes
		or techniques with similarly aliased units, the latter ones
		will be ignored. If there are no aliased units, an empty table
		is returned.
		@param materialName The name of an existing material to inspect
		                    for texture aliases.
		@return A table mapping texture aliases to texture names. E.g.
		    <code>{["Diffuse"]="ShinyRedCar.png"}</code>, or an empty
		    table if none are found.
	*/
	function getMaterialTextures(materialName):Table;
	
	/**
		Paint onto the terrain splat coverage map. This will update
		the corresponding alpha channel(s) on the area covered by the
		brush parameters. The terrain splat page must have the given
		texture in one of its 4 splat aliases (Splat0 through Splat3).
		<p>
		The brush is a circular shape with the weight and falloff parameters
		controlling the density of values applies within the circle formed
		by the brush.
		<p>
		This does not automatically write the resulting texture modification(s)
		to disk. You must explicitly save the results using
		System.writeTextureToFile() if you want the changes to persist.
		
		@param x The X coordinate center of the brush in world coordinates.
		@param z The Z coordinate center of the brush in world coordinates.
		@param brushRadius The radius of the brush in world coordinates.
		@param brushWeight A value between 0.0 and 1.0 indicating "opacity" or
		    "hardness" (i.e. how much of the value to apply). Setting this
		    to something like 0.5 causes a blending effect to be used.
		@param brushFalloff A value between 1.0 and 10.0 that controls the
			"sharpness" of the brush from the center toward its circumferance.
			1.0 is a smooth transition from center to outer edge, whereas
			higher values have increasingly sharp falloff (and are more solid
			in the middle).
		@param texture The splat texture channel to "paint" in to.
	*/
	function terrainSplatPaint(x:Float, z:Float,
		brushRadius:Float, brushWeight:Float, brushFalloff:Float,
		texture:String);
	
	/**
		Paint the terrain heightmap values according to a predefined function.
		This will modify the height values in the terrain according to
		one of the following functions:
		<dl>
		<dt>0 - ADD</dt><dd>Add the function value to the existing height
		  value. This is used to make relative (+ or -) changes.</dd>
		<dt>1 - ASSIGN</dt><dd>Overwrite the old value with the new.</dd>
		<dt>2 - SMOOTH</dt><dd>Perform a blurring operation of the values
		  within the brush. The function value in this case indicates the
		  level of blurring to perform (higher values will be blurred more).</dd>
		<dt>3 - NOISE</dt><dd>Add random noise to the current height value
		  in the terrain.</dd>
		</dl>
		<p>
		The brush is a circular shape with the weight and falloff parameters
		controlling the density of values applies within the circle formed
		by the brush.
		<p>
		This does not automatically write the resulting height modification(s)
		to disk. You must explicitly save the results using
		System.writeTerrainHeightToFile() if you want the changes to persist.
		
		@param x The X coordinate center of the brush in world coordinates.
		@param z The Z coordinate center of the brush in world coordinates.
		@param brushRadius The radius of the brush in world coordinates.
		@param brushWeight A value between 0.0 and 1.0 indicating "opacity" or
		    "hardness" (i.e. how much of the value to apply). Setting this
		    to something like 0.5 causes a blending effect to be used.
		@param brushFalloff A value between 1.0 and 10.0 that controls the
			"sharpness" of the brush from the center toward its circumferance.
			1.0 is a smooth transition from center to outer edge, whereas
			higher values have increasingly sharp falloff (and are more solid
			in the middle).
		@param functionType One of the predefined brush "types" (see above).
		@param functionValue A value that will be passed to the brush "type"
		    whose meaning depends on the type of brush selected.
	*/
	function terrainHeightPaint(x:Float, z:Float,
		brushRadius:Float, brushWeight:Float, brushFalloff:Float,
		functionType:Integer, functionValue);
	
	/**
		Load a composition into the currently running player. This attempts
		to load a new supporting (i.e. non-root composition) into the currently
		running player. It uses the media cache to locate, fetch, validate,
		and eventually load the contents of the composition with the specified
		name. Because of the inherently asynchronous nature of dealing with
		media over the internet, the contents of the composition may not be
		immediately available.
		<p>
		If you need to know when the contents of the composition are avilable
		and can be used, you must do the following:
		<ol>
		<li>If the return value of this function is true, then the composition
		was already in the cache and was able to be loaded immediately. No
		further processing need be done, the composition was loaded and is
		ready.</li>
		<li>If the return value is false, it was submitted to the asynchronous
		load system, so you must have registered a listener to the
		<code>_cache</code> media cache and listen for one of the completion
		events (either  <code>onComplete</code> or <code>onError</code>). And
		react accordingly once it has been fetched. Note, compositions are not
		actually available until <b>the next frame</b> after a completion
		event.</li>
		<li>Alternately, provide your own asynchronous notification scheme
		by having a well-known object broadcast a message from the <i>loaded</i>
		composition through its _init_.nut script. For example, in the loading
		composition, you might have:
		<pre>
		// Create a well-known message broadcaster. The loading composition
		// will use this to tell the loader that it is ready.
		gLoadNotifier = MessageBroadcaster();
		// Add a listener in the loader that will respond to the composition
		// load events.
		gLoadNotifier.addListener({
			function onCompositionLoaded(name)
			{
			   print("Composition loaded: " + name);
			}});
		</pre>
		<p>
		And, in the loading composition (or compositions if you use multiple
		support compositions), you might have the following in _init_.nut:
		<pre>
		// ... some initialization stuff ...
		
		// Use the well-known notification object to tell the loader
		// we are done loading and are fully initialized.
		gLoadNotifier.broadcastMessage("onCompositionLoaded", "MyLoadedComposition");
		</pre>
		
		</li>
		</ol>

		@param location
			The location of the composition to load. The name should be
			relative to the current code base.

		@returns True if the composition was immediately available and loaded
			successfully. Otherwise, the load request is submitted to the
			media cache and loading occurs asynchronously.
	*/
	static function loadAsync(location:String):Boolean;
	
	
	/**
		Set the debugging flag that will cause more verbose information
		to be logged while compositions are loading. (Default: false)
	*/
	static function setLogLoading(enabled:Boolean);
	
	/**
		Load a composition file from local storage. Once a composition
		has been fetched (using a MediaCache), it can be loaded into
		the player. The loadAsync() function does this automatically
		if it's fetched successfully, but sometimes you need more control
		of the download process (such as to receive notifications when
		there is a problem or to trigger an event when it's ready). In
		this case, you need to monitor the MediaCache yourself and in
		addition to whatever else you might be doing, you should call
		this function in the onComplete() event handler, with the
		media name and file handle provided in that routine.
		
		@param location The code-base relative path to the composition
			that is being loaded.
		@param filename The file handle for the given composition (as
			obtained from MediaCache's <code>onComplete</code> event).
			
		@return True if loaded successfully, or false if not.
	*/
	static function load(location:String, filename:String):Boolean;
	
	
	// Composition events are sent only to the root composition.
	static events =
	{
		/**
			This is triggered every frame, before any other scripting
			logic is triggered (input events, networking, etc). This
			is also prior to rendering the current frame.
		*/
		onEnterFrame = function(){}
		
		/**
			This is triggered every frame, after all scripting events
			have been triggered, and immediately before rendering the
			current frame.
		*/
		onExitFrame = function(){}
		
		/**
			This is triggered every frame after all render operations have
			been submitted to the GPU, but before the next frame has started.
			By putting some computational tasks here, you can parallelize some
			CPU-bound operations and increase overall throughput.
		*/
		onNextFrame = function(){}

		/**
			Triggered when the screen is resized. This can happen in response
			to the user changing a setting or toggling fullscreen on or off.
			<p>
			You should use this event to re-initialize or change any element
			that might be screen size dependent such as: UI elements, fonts,
			buttons, etc. Use Screen.getWidth() and Screen.getHeight() as needed.
		*/
		onScreenResize = function(){}
		
		
		/**
			A terrain page (set of terrain tiles) has fully loaded. This
			is called after the last tile in the given page has been
			loaded.
			<p>
			You should probably not initialize extra scene elements in
			this event, but rather in onTerrainTileLoaded(), as that
			provides a much finer grained (and staggered) opportunity
			for loading scene data.
			<p>
			The page coordinates are indexes into the virtual 2D "page map" 
			of all pages (as defined in the terrain configuration file).
			
			@param pageX The X axis page coordinate of the page.
			@param pageZ The Z axis page coordinate of the page.
			@param bounds The axis aligned bounding box for the page extents.
		*/
		onTerrainPageLoaded = function(pageX:Integer, pageZ:Integer,
					bounds:AxisAlignedBox){}

		/**
			A terrain tile has fully loaded. This is called after the
			tile has been created and added to the scene.
			<p>
			The intended purpose of this callback is to allow initialization
			of scene geometry after the terrain has been built. For instance,
			loading .mesh files, setting up static geometry, or other paremeters
			based on this small "atomic" chunk of terrain.
			<p>
			The page coordinates are indexes into the virtual 2D "page map" 
			of all pages (as defined in the terrain configuration file).
			And the tile coordinates index into that page (each page is a 2D
			grid of tiles).
			
			@param pageX The X axis page coordinate of the page.
			@param pageZ The Z axis page coordinate of the page.
			@param tileX The X index (within the page) identifying the tile.
			@param tileZ The Z index (within the page) identifying the tile.
			@param bounds The axis aligned bounding box containing the
					tile's extents.
		*/
		onTerrainTileLoaded = function(pageX:Integer, pageZ:Integer,
					tileX:Integer, tileZ:Integer,
					bounds:AxisAlignedBox){}
		
		/**
			A terrain page (set of terrain tiles) has fully unloaded. This
			is called after the last tile in the given page has been
			unloaded.
			<p>
			If you allocated any scene objects during a page loaded event,
			you should probably deallocate them in either a tile unloaded
			event or as a last resort, in this method.
			<p>
			The page coordinates are indexes into the virtual 2D "page map" 
			of all pages (as defined in the terrain configuration file).
			
			@param pageX The X axis page coordinate of the page.
			@param pageZ The Z axis page coordinate of the page.
		*/
		onTerrainPageUnloaded = function(pageX:Integer, pageZ:Integer);
		
		/**
		*/
		onTerrainTileUnloaded = function(pageX:Integer, pageZ:Integer);

		// Ph. II
		//onGainFocus = function(){};		// The composition/player has gained keyboard focus
		//onLoseFocus = function(){};		// The composition/player has lost keyboard focus
	};
}

/**
	A 2d frame capable of responding to user input and/or displaying
	text and graphics.
*/
class Widget extends UI_Element
{
	/**
		Obtain a reference to an overlay element within the scene. The
		element must exist, or this will throw an error.
	*/
	constructor(name:String);

	/**
		Get the name of this widget. The name is specified in the
		constructor (or set via a dup() operation).
	*/
	function getName():String;

	function isVisible():Boolean;
	function setVisible(value:Boolean);

	function isEnabled():Boolean;
	function setEnabled(value:Boolean);

	function setSize(width,height);
	function setPosition(left,top);
	function getWidth():Float;
	function getHeight():Float;
	function getLeft():Float;
	function getTop():Float;

	function getMaterialName():String;
	function setMaterialName(material:String);

	function getColor():Color;
	function setColor(color:Color);

	function setParam(name:String, value:String);
	function getParam(name:String):String;
	
	/**
		Get whether this widget is a container widget. Only containers
		may contain other child widgets. (Whether or not a widget is
		a container depends on the definition in the overlay file.)
	*/
	function isContainer():Boolean;
	
	/**
		Test whether any of a container's children are interested in
		receiving input events. When they aren't, they are not included
		in things like pick operations.
		<p>
		If the widget is not a container, this is always false.
	*/
	function isChildProcessingEvents():Boolean;
	/**
		Set whether the children of a container are interested in
		processing input events (such as mouse clicks). When this is
		off, the children are not considered when resolving input
		events (thus, you can explicitly disable events for child
		elements/widgets even if they normally generate/handle them).
		<p>
		This is typically used in grouping elements into a more complex
		"meta" widget that handles things at a higher level.
		<p>
		If the widget is not a container, this does nothing.
	*/
	function setChildProcessingEvents(value:Boolean);

	/**
		Get a reference to the parent of this element, if any. If
		the element is not currently in a parent container, this
		will return <code>null</code>.
	*/
	function getParent():Widget;
	
	/**
		Put this widget in the parent's child list (or remove it
		from its current parent). If non-null, the widget must
		be a container widget (or an error will be generated),
		and this widget will be added as a child of the specified
		parent. If <code>parentWidget</code> is <code>null</code>,
		then this widget is removed from its existing parent (if
		any).
		<p>
		Note, having a parentless widget may cause it to not be
		visible.
		<p>
		Also, coordinate systems are generally relative, so if you
		reparent a widget, be aware that the position (and/or size)
		may have to also be adjusted to keep thing sane.
	*/
	function setParent(parentWidget:Widget);

	/**
		Get the current number of child elements this widget has.
		If not a container, this always returns 0.
	*/
	function getChildCount():Integer;
	
	/**
		Get a reference to a specific child in this container widget.
		This will throw an error if the widget is not a container.
	*/
	function getChild(index:Integer):Widget;

	/**
		Create a new widget by cloning a template definition. The
		template is defined within an .overlay file and can be 
		referenced by name. The instance name is the same as in
		dup(), that is, a prefix for any and all new elements/widgets
		created by this process.
		<p>
		Note: this is a static method of Widget, and hence, can (and
		should be called using the class name, not an instance of the
		class). E.g. Widget.createFromTemplate(), not
		mywidget.createFromTemplate().
		<p>
		Example:
		<pre>
		local w = Widget.createFromTemplate("UI/Button", "MyButton");
		// Creates MyButton/UI/Button and any other elements stored in
		// the UI/Button template definition.
		</pre>
		<p>
		The template must exist or this will throw an error. The newly
		created instances must all have unique names, so choose an
		<code>instanceName</code> accordingly.
	*/
	static function createFromTemplate(templateName:String, instanceName:String):Widget;
	
	/**
		Create a new widget by cloning the contents of this one. This
		is used to instantiate widgets from a templatized version of
		it. For instance, a UI/Button template widget could be 
		dup()'d with the name "myButton". This will recreate the
		entire hierarchy represented by -this- widget (even if it's
		only a single element) by prefixing the new instance name
		to all the elements.
		<p>
		So, for instance: if UI/Button contains elements UI/Button,
		UI/Button/BG, and UI/Button/Label, then the new widget will
		contain: myButton/UI/Button, myButton/UI/Button/BG, and
		myButton/UI/Button/Label.
		<p>
		Currently, the newly created widget (OverlayElement tree)
      is created "detached" from any overlay, and thus, will not
      be visible. You must explicitly place it on an overlay using
      setOverlay().
	*/
	function dup(newInstanceName:String):Widget;

   /**
		Destroy this widget and all of its child elements (if any). You
      must not reference this widget after this call returns as it
      will have been destroyed in the engine.
		<p>
      You can only destroy widgets that have been created via dup().
		If you want other widgets (that "wrap" predefined elements),
      to be offscreen, you may hide by calling setVisible(false).
   */
	function destroy();

   /**
		Set or change the overlay that contains this widget. Only container
      widgets may have their overlay value set. Also, the overlay must
      already exist. If either of these conditions are not met, this
		returns false, otherwise true.
   */
	function setOverlay(overlayName:String):Boolean;
}

/**
	An abstract base class for other input events (such as mouse input
	or key presses).
*/
class InputEvent
{
	// Inspect the various state conditions for this event.
	function isAltDown():Boolean;
	function isShiftDown():Boolean;
	function isControlDown():Boolean;
	function isLButtonDown():Boolean;
	function isMButtonDown():Boolean;
	function isRButtonDown():Boolean;
	
	/**
		Check if this input event has been "consumed".
		
		@see #consume()
	*/
	function isConsumed():Boolean;
	
	/**
		Consume this event, preventing other UI elements from
		receiving it and performing any further processing.
		Calling this from a key event handler, for instance,
		will prevent that key from being handled in a higher-level
		element, such as the root composition.
	*/
	function consume();
	
	/**
		Get the widget that this event is currently being triggered
		for. This may or may not be the "originating" widget. It's
		exposed so that listeners can be re-used and act on a specific
		instance in their implementation methods.
	*/
	function getWidget():Widget;
}

/**
	An input event generated by the keyboard.
*/
class KeyEvent extends InputEvent
{
	keyCode = 0; // The Key.VK_* keycode
}

/**
	An input event generated by a mouse press or movement.
*/
class MouseEvent extends InputEvent
{
	static LBUTTON = 1	// Left button
	static MBUTTON = 2	// Middle button
	static RBUTTON = 3	// Right button
	
	/**
		The horizontal position of the cursor in -client- space
		(i.e. the left edge of the UI element receiving this event)
		pixels. The top left is 0,0 and the bottom right is width-1, height-1.
	*/
	x = 0;
	
	/**
		The vertical position of the cursor in -client- space
		(i.e. the top edge, of the UI element receiving this event)
		pixels. The top left is 0,0 and the bottom right is width-1, height-1.
	*/
	y = 0;

	/**
		For button events, which button (LBUTTON, MBUTTON, etc)
		has been pressed/released.
	*/
	button = 0;
	
	/**
		Vertical movement units. (For things like mousewheel).
		The value is undefined for those events it doesn't
		apply to (e.g. MousePressed).
	*/
	units_v = 0;
	
	/**
		The number of "clicks" associated with a mouse down event.
		I.e. for a single click, this will be 1, for a double, it
		will be 2. (Triple clicks or more are not supported currently.)
	*/
	clickCount = 0;
}

/**
	Global keyboard interface. Use this to inspect the keyboard
	state and to translate key code constants into more meaningful
	(i.e. human-readable) representations.
*/
class Key
{
	/**
		Get a human-readable description of the key code. For
		instance VK_ENTER will return "Enter".
	*/
	static function getText(keyCode:Integer);
	
	/**
		Test if a key is currently pressed. Returns true if the
		key is pressed, and false if not.
	*/
	static function isDown(keyCode:Integer);
	
	/**
		Test if a toggleable key (CAPS lock, etc) is currently
		enabled/active. 
	*/
	static function isToggled(keyCode:Integer);

	static CHAR_UNDEFINED = 0xffff;

	// KeyCode constants
	static VK_BACK =         0x08;
	static VK_TAB =          0x09;
	static VK_CLEAR =        0x0C;
	static VK_ENTER =        0x0D;
	static VK_SHIFT =        0x10;
	static VK_CONTROL =      0x11;
	static VK_ALT =          0x12;
	static VK_PAUSE =        0x13;
	static VK_CAPS =         0x14;

	static VK_ESCAPE =       0x1B;

	static VK_CONVERT =      0x1C;
	static VK_NONCONVERT =   0x1D;
	static VK_ACCEPT =       0x1E;
	static VK_MODECHANGE =   0x1F;

	static VK_SPACE =        0x20;
	static VK_PAGEUP =       0x21;
	static VK_PAGEDOWN =     0x22;
	static VK_END =          0x23;
	static VK_HOME =         0x24;
	static VK_LEFT =         0x25;
	static VK_UP =           0x26;
	static VK_RIGHT =        0x27;
	static VK_DOWN =         0x28;
	static VK_SELECT =       0x29;
	static VK_PRINT =        0x2A;
	static VK_EXECUTE =      0x2B;
	static VK_SNAPSHOT =     0x2C;
	static VK_INSERT =       0x2D;
	static VK_DELETE =       0x2E;
	static VK_HELP =         0x2F;

	static VK_0 = '0';
	static VK_1 = '1';
	static VK_2 = '2';
	static VK_3 = '3';
	static VK_4 = '4';
	static VK_5 = '5';
	static VK_6 = '6';
	static VK_7 = '7';
	static VK_8 = '8';
	static VK_9 = '9';
	static VK_A = 'A';
	static VK_B = 'B';
	static VK_C = 'C';
	static VK_D = 'D';
	static VK_E = 'E';
	static VK_F = 'F';
	static VK_G = 'G';
	static VK_H = 'H';
	static VK_I = 'I';
	static VK_J = 'J';
	static VK_K = 'K';
	static VK_L = 'L';
	static VK_M = 'M';
	static VK_N = 'N';
	static VK_O = 'O';
	static VK_P = 'P';
	static VK_Q = 'Q';
	static VK_R = 'R';
	static VK_S = 'S';
	static VK_T = 'T';
	static VK_U = 'U';
	static VK_V = 'V';
	static VK_W = 'W';
	static VK_X = 'X';
	static VK_Y = 'Y';
	static VK_Z = 'Z';

	static VK_LWIN =         0x5B;
	static VK_RWIN =         0x5C;
	static VK_APPS =         0x5D;

	static VK_SLEEP =        0x5F;

	static VK_NUMPAD0 =      0x60;
	static VK_NUMPAD1 =      0x61;
	static VK_NUMPAD2 =      0x62;
	static VK_NUMPAD3 =      0x63;
	static VK_NUMPAD4 =      0x64;
	static VK_NUMPAD5 =      0x65;
	static VK_NUMPAD6 =      0x66;
	static VK_NUMPAD7 =      0x67;
	static VK_NUMPAD8 =      0x68;
	static VK_NUMPAD9 =      0x69;
	static VK_MULTIPLY =     0x6A;
	static VK_ADD =          0x6B;
	static VK_SEPARATOR =    0x6C;
	static VK_SUBTRACT =     0x6D;
	static VK_DECIMAL =      0x6E;
	static VK_DIVIDE =       0x6F;
	static VK_F1 =           0x70;
	static VK_F2 =           0x71;
	static VK_F3 =           0x72;
	static VK_F4 =           0x73;
	static VK_F5 =           0x74;
	static VK_F6 =           0x75;
	static VK_F7 =           0x76;
	static VK_F8 =           0x77;
	static VK_F9 =           0x78;
	static VK_F10 =          0x79;
	static VK_F11 =          0x7A;
	static VK_F12 =          0x7B;
	static VK_NUMLOCK =      0x90;
	static VK_SCROLL =       0x91;
	
	static VK_SEMICOLON =    0xBA;
	static VK_EQUALS =       0xBB;
	static VK_COMMA =        0xBC;
	static VK_DASH =         0xBD;
	static VK_PERIOD =       0xBE;
	static VK_SLASH =        0xBF;
	static VK_GRAVE =        0xC0; 
	static VK_LBRACKET =     0xDB;
	static VK_BACKSLASH =    0xDC;
	static VK_RBRACKET =     0xDD;
	static VK_SINGLE_QUOTE = 0xDE;
}

/**
	An interface to the screen (well, the viewport the player is running in).
*/
class Screen
{
	/**
		Get the current width of the screen in pixels.
	*/
	static function getWidth():Integer;
	/**
		Get the current height of the screen in pixels.
	*/
	static function getHeight():Integer;
	/**
		Get whether the player is currently running in fullscreen mode
		or not.
	*/
	static function isFullscreen():Boolean;
	/**
		Attempt to toggle in to or out of fullscreen mode. This will
		attempt to honor the user's preferences with respect to
		resolution and color depth (as originally configured or auto-
		detected) when going into fullscreen mode.
		<p>
		NOTE: Not implemented yet.
	*/
//	function setFullscreen(value:Boolean);

	/**
		Use the specified font to calculate the extents (in pixels)
		for the given string of text. If the text has newline characters
		in it ('\n'), then this will report a height required to render
		the lines of text.

		@param text The text that should be used to calculate size.
		@param font The name of the font to use when calculating size
					(if the font doesn't exist, an error will be thrown).
		@param size The font size (in pixels) to use in the calculation.

		@return A table with the following values:
		<ol>
		<li>width - The maximum width, in pixels, of all lines
				of text.</li>
		<li>height - The maximum height, in pixels, of all lines
				of text.</li>
		</ol>

	*/
	static function getTextMetrics(text:String, font:String, size:Float):Table;

	/**
		Get whether the mouse cursor is currently visible.
	*/
//	static function isCursorVisible():Boolean;
	/**
		Set the visibility of the mouse cursor. Returns the
		-previous- visibility state of the cursor.
	*/
//	static function setCursorVisible(value:Boolean):Boolean;

	/**
		Enable/disable the mouse input event capturing for the screen. When captured,
		the player window will receive all mouse events while the window is focused,
		even if they are outside the boundaries of the player window.
		
		@param filter If non-null, this is the element that will receive the mouse
			events, to the exclusion of all other elements. Otherwise, a null value
			will disable mouse capturing, returning normal behavior.
	*/
	static function setMouseCapture(filter:UI_Element);

	/**
		Get the current location of the mouse cursor relative to the rendering window
		of the player (i.e. 0,0 is the upper left corner of the render window). Note
		that for non-fullscreen windows, this could well be negative and/or outside
		the rendering window.
		@return A table containing an 'x' and 'y' field with the position of each.
	*/
	static function getCursorPos():Table;

	/**
		Set the cursor position on screen relative to the player's rendering window.
		(See getCursorPos()).
	*/
	static function setCursorPos(x:Integer, y:Integer);
	
	/**
		Sets whether or not the cursor should be visible
	*/
	static function setCursorVisible(which:Boolean);

	/**
		Get the background color for the rendering window (Default: Black)
	*/
	static function getBackgroundColor():Color;
	
	/**
		Set the background color for all viewports in the rendering window.
		This is the "clear" color used when rendering a new frame.
	*/
	static function setBackgroundColor(color:Color);
	
	/**
		Test if the specified overlay is currently visible.
	*/
	static function isOverlayVisible(name:String):Boolean;
	
	/**
		Set whether the specified overlay (FPS and profiling info) is drawn
		on the screen or not.
		<p>
		There are a couple predefined overlays that can be enabled (others
		must be defined in the composition using *.overlay files):
		<ol>
		<li>Core/Debug -- The player's debugging information (current FPS, etc)</li>
		<li>Core/Profiling -- The player's current profiling information</li>
		</ol>
	*/
	static function setOverlayVisible(name:String, value:Boolean);
	
	
	/**
		Add a scene node (and all implicitly all its children) to
		a 2D overlay layer. The scene node's coordinate system then becomes
		relative to the active camera in a rendering scene (instead of the
		root scene node). The scene node should not be attached to the
		regular scene in this case (but it does not enforce this).
		<p>
		Once added to an overlay, it will be rendered according to the
		Z-ordering of the overlay it is added to.
		<p>
		A major caveat with this technique is managing the depth buffer.
		Because many (if not all) 2D elements in an overlay do not render
		with depth checking enabled, it's possible to have 3D objects
		added using this method poke through other 2D elements in -any-
		overlay. This is entirely dependent on the setup of your 2D/3D
		overlay elements and varies from object to object.
		
		@param overlay
			The name of the overlay to add to.
		@param node
			The SceneNode to add to the overlay.
	*/
	static function addOverlaySceneNode(overlay:String, node:SceneNode);
	
	/**
		Remove a previously added SceneNode from an overlay. If the
		scene node is not a part of the overlay, it is ignored. Once
		removed, the scene node is in a "detached" state (i.e. not 
		attached to the root scene node) and must be reattached to
		a valid scene node to become visible/renderable again.
	*/
	static function removeOverlaySceneNode(overlay:String, node:SceneNode);
	
	/**
		Convert a 3d world position into its corresponding 2d position
		in screen coordinates based on the current camera transformation.
		
		@param pos The 3d position in world coordinates to translate into 
					screen coordinates.
		@return A table with integer 'x' and 'y' fields representing the
		translated values. It also includes the 'z' coordinate in a 
		[0,1] range which can be used to see if the returned
		point is behind the camera or not ('z' will be outside this range
		when not within the camera's FoV).
	*/
	static function worldToScreen(pos:Vector3):Table;
	
	/**
		Get a ray that "passes" through the specified screen coordinate
		based on the camera's current transformation and projection.
		This ray can be used to implement "picking".
		<p>
		This returns a table containing a 'origin' and 'dir' Vector3
		members that make up the ray.
		
		@param x The x screen coordinate.
		@param y The y screen coordinate.
		
		@return A Table with <code>origin</code> and <code>dir</code>
			members that hold each part of the calculated ray.
	*/
	static function getViewportRay(x:Integer, y:Integer):Table;
	
	/**
		Save the contents of the current render window to a timestampped
		file. The file will be in the executable directory and named:
		(Prefix)-(Timestamp).png where the default prefix is "Screenshot".
		
		@param prefix (Optional) If provided, the file will have the specified
						prefix. Default: "Screenshot-"
	*/
	static function saveScreenshot(...);
	
	/**
		Set the title of the player's render window (if it has one). If
		the player is currently embedded in an environment where this does
		not apply, it may be ignored.
		
		@param title The text to use as the new window title.
	*/
	static function setTitle(title:String);

    /**
       @param filter An instance or table containing functions to run
                       on each input event (onMouseMoved, etc).  The
                       filter is given the event in screen
                       coordinates.  If the filter consumes the event,
                       the event is consumed right there before it
                       gets to any normal event processing.
    */
    static function setInputEventFilter(...);
}

/**
	ByteBuffers encapsulated a fixed size block of data and
	provide a portable mechanism for encoding native data values. This
	is used by the network transports to send and recieve binary messages
	over the wire, but the actual implementation is generic and can
	be used anywhere.
	<p>
	The buffer has two pointers along with the allocated memory. These
	pointers point to the "front" and "back" of the buffer and are used
	to sequentially write chunks of binary data. At any given time, the
	buffer can be considered to be in "write" mode or "read" mode, and
	there are methods to switch between the two. However, this mode is
	only semantic, there is no internal flag indicating what state the
	buffer is in, it's determined by how you use it in code.
	<p>
	For example, we can create a new buffer and it is initially 
	in a "write" state (ready to be written to) since the <i>position</i>
	(i.e. the "front") is at 0, and the <i>limit</i> (i.e. the "back") is
	set to the full capacity. It is ready to receive up to X bytes (where
	X is the entire size of the buffer). We can then write something to
	it, flip it (put it into "read" mode), and then read some data out
	again:
	<pre>
		local buf = ByteBuffer(10);
		buf.putInteger(123);
		// Position = 4 now since an integer is 4 bytes
		buf.putByte(45);
		// Position = 5 (a byte is in fact only a single byte in size ;))
		// Now, let's flip it and put it into "read" mode.
		buf.flip();
		// At this point, position is back at 0, but limit (the 'back')
		// is set to 5, since there were only 5 bytes previously written.
		buf.getInteger(); // Read the integer 123; position = 4, limit = 5
		buf.getByte(); // Read the byte 45; position = 5, limit = 5.
		// At this point, there are no more bytes ready to be read, if
		// we try again, it will throw an error saying so. We can put
		// it back into "write" mode either by flip()'ing again, or
		// we can use the special compact() call, which does the
		// same thing, but moves any left over data to the front
		// of the buffer as well, giving us the maximum amount of
		// room to continue writing to.
		buf.compact(); // Copy any remaining data to front (if any), and flip
		// Position = 0, limit = 10 (the full capacity)
	</pre>
	<p>
	It can take a small bit of time to get the hang of how this works, but
	once you do, it's quite easy to use.
*/
class ByteBuffer
{
	/**
		Create a new ByteBuffer and allocate a block of memory.
	*/
	constructor(nbytes:Integer);
	
	/**
		Return the number of bytes remaining to be read or the amount of
		bytes left that can be written to. I.e. limit - position.
	*/
	function remaining():Integer;
	
	/**
		Return the current read/write position (if called with no arguments),
		or set it if called with a single integer. The new position must be
		<= limit and >= 0.
	*/
	function position(...):Integer;
	
	/**
		Return the current read/write limit (if called with no arguments),
		or set it if called with a single integer. The new limit must be
		<= capacity and >= 0. If the position would be out of range after
		setting the limit, it is set to be equal to the limit.
	*/
	function limit(...):Integer;
	
	/**
		Return the capacity of this buffer. (Determined at time of construction)
	*/
	function capacity():Integer;
	
	/**
		Clear the buffer. Sets position to 0, and limit to capacity.
	*/
	function clear():ByteBuffer;
	
	/**
		"Flip" the buffer's read/write state. Sets the limit to the current position,
		and moves the position to 0.
	*/
	function flip():ByteBuffer;
	
	/**
		Move any remaining bytes to the front of the buffer, and flip into "write" state.
		If position < limit, then the difference is copied into the front of the buffer
		(position 0). The limit is set to capacity and the position to the end of any
		bytes still remaining after compacting.
	*/
	function compact():ByteBuffer;
	
	/**
		Sets the position to 0.
	*/
	function rewind():ByteBuffer;
	
	/**
		Read a signed 4 byte integer from the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function getInteger():Integer;
	
	/**
		Read a signed 2 byte integer from the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function getShort():Integer;
	
	/**
		Read a signed 1 byte integer from the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function getByte():Integer;
	
	/**
		Read a signed 1 byte integer from the buffer and interpret any non-zero value
		as true and zero as false.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function getBoolean():Boolean;
	
	/**
		Read a length-encoded string from the buffer. (Encoding is still
		ASCII for the time being, not UTF.)
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function getStringUTF():String;
	
	/**
		Read the buffer until either the first '\0' character is
		encountered or the remaining bytes are consumed. If '\0'
		terminates this string, it is not consumed as part of the
		operation.
	*/
	function getString():String;
	
	/**
		Read a 4 byte floating point number from the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function getFloat():Float;
	

	/**
		Write a signed 4 byte integer to the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function putInteger(value:Integer);
	
	/**
		Write a signed 2 byte integer (least significant bits) to the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function putShort(value:Integer);
	
	/**
		Write a signed 1 byte integer (least significant bits) to the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function putByte(value:Integer);
	
	/**
		Write a signed 2 byte integer (least significant bits) to the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	
	function putBoolean(value:Boolean);
	/**
		Write a length-encoded string to the buffer. (The encoding is ASCII
		currently, not UTF, despite the name of this method.)
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function putStringUTF(value:String);
	
	/**
		Write the contents of the string into the buffer, not including
		a terminating '\0' character. (Contrast to getString() which reads
		until it finds a terminating character.)
	*/
	function putString(value:String);
	
	/**
		Write a 4-byte floating point value into the buffer.
		<p>
		This may throw an error if there are not enough bytes in the buffer,
		otherwise it advances the position.
	*/
	function putFloat(value:Float);
	
	/**
		Write the contents of the given buffer into this one, up to the
		minimum of the remaining bytes in each. That is, fill this buffer
		up until it's full or the other one is empty, whichever comes first.
	*/
	function putBuffer(buf:ByteBuffer);
}

/**
	A socket-based network connection.
*/
class Socket extends MessageBroadcaster
{
	/**
		Begin a connection request to the specified host and port.
		If the socket is already connected, or some other error
		occurs while trying to connect, this returns false.
		<p>
		Note: a socket is not necessarily ready for reading/writing
		after calling this method, even when it returns true. Writes
		will be buffered until it -is- ready, but it is something to
		be aware of. If you must know exactly when it is fully connected,
		use an event listener.
	*/
	function connect(host:String, port:Integer):Boolean;
	
	/**
		Is the connection currently open? This is set when a connection
		is first requested and lasts until either explicitly closed
		or closed via a network error or timeout.
	*/
	function isOpen():Boolean;
	
	/**
		Has a connection been established. Note, this is not the
		same thing as having the socket open. There is a time
		period between opening a socket and having a connection
		established where isOpen() and isConnected() will report
		two different things.
	*/
	function isConnected():Boolean;
	
	/**
		Disconnect and close down the connection normally.
	*/
	function close();
	
	/**
		Send one or more bytes on the connection. It will attempt
		to write as much of the buffer as will fit in the internal
		write buffer for this socket. It returns true if all of the
		bytes in the specified buffer were successfully written to
		the socket.
	*/
	function write(buf:ByteBuffer):Boolean;
	/**
		Write the bytes of the given string to the socket. This is
		a convenience method so a ByteBuffer doesn't have to be
		used to wrap the string first. (Useful for string based
		connections.)
	*/
	function writeStringBytes(str:String):Boolean;
	
	/**
		Close the writing portion of this socket. This does not
		necessarily close the socket entirely, it may linger
		trying to read any remaining data. This generally
		signals to the server that you are done with this socket
		and it should close down your connection after sending
		any remaining data to you (if any). 
	*/
	function shutdownOutput();
	
	/**
		Close the reading portion of this socket. (Not usually
		explicitly needed.)
	*/
	function shutdownInput();
	
	/**
		Get the last IO error to occur on this socket, if any.
	*/
	function getLastError():String;
	
	static events = 
	{
		/**
			Data has been received over the network. The data buffer
			contains the bytes received (up to the maximum buffer size,
			currently 8192). If you do not consume all of the bytes here,
			they will remain in the buffer for the next time
			data is received.
			<p>
			WARNING: If you don't leave enough room in the read buffer
			by consuming all/most of the bytes, you risk having the
			socket stall (never being able to read any more).
			<p>
			Do not register more than one listener for this event.
		*/
		onRecv = function(socket, data:ByteBuffer) {}
		/**
			A connection has been started, but not fully established
			yet. The socket is not yet ready to read or write from.
			This is purely informational.
		*/
		onConnectStart = function(socket){}
		/**
			A connection has been fully established and is ready to
			read and/or write.
		*/
		onConnect = function(socket){}
		/**
			A connection had been started, but the specified timeout
			(default 30 seconds) has elapsed and the socket is being
			forcefully closed.
		*/
		onTimeout = function(socket){}
		/**
			A connection has been closed normally after being connected.
		*/
		onClose = function(socket){}
		/**
			An IO error has occurred. Use Socket.getLastError() to
			inspect the reason. The socket is closed by the system
			before calling this event.
		*/
		onError = function(socket){}
	};
}


/**
	Manage a local media cache. This is capable of retrieving
	files from a remote server and making sure they are up to
	date on the local machine. It also provides a way of receiving
	asynchronous notification events when that media must be
	fetched.
	<p>
	TODO: Add eviction policy/policies (e.g. max size/age) to
	maintain local cache size within limits.
*/
class MediaCache extends MessageBroadcaster
{
	static events =
	{
		/**
			A file has been retrieved (or was already present),
			and can be read from using the filename given here.
			Note: you cannot rely on the name of the cached file
			being consistent (or even intelligible). The cache
			system tries to preserve a meaningful name, but makes
			no guarantees about it. In general, you should not
			need to actually see what the transient filename is,
			since all references should be done using "media" names.
		*/
		onComplete = function(media:String, cachedFile:String) {}
		
		/**
			An error has occurred during the download or update
			of a media file. The error is provided in the <code>
			error</code> parameter. The cache does -not- have a
			copy of the media file after this event (any old
			reference will have been removed).
		*/
		onError = function(media:String, error:String) {}
		
		/**
			The media file has physically started to download
			by being moved to the front of the queue.
		*/
		onStart = function(media:String) {}
		
		/**
			The media file is being downloaded and making some
			progress along the way. This is primarily of use
			only to give feedback to the user.
		*/
		onProgress = function(media:String, loadedBytes, totalBytes) {}
	}
	
	/**
		Create a cache based in the specified local directory,
		and that references a specified "base" URL. This works
		as follows:
		<ol>
		<li>A request is made by this local cache to retrieve a
		    file using a relative name (e.g. "some/file/foo.xml").</li>
		<li>The cache first checks if it is present in the local
		    store, if it is, and the cache deems it "fresh" enough
		    to return immediately, it does so (via the "onComplete"
		    event).</li>
		<li>If the file is not in the cache, it uses the baseURL
		    to locate the file and begin downloading it via an
		    internal queue. Once the file has moved to the front
		    of the queue and actually started downloading, the
		    "onStart" event is triggered. During the download,
		    the "onProgress" event may trigger (possibly multiple
		    times) to give feedback to the listener of forward
		    progress.</li>
		<li>Once the file has been downloaded, the cache is updated,
		    and it returns a "onComplete" event to any registered
		    callback.</li>
		<li>If at any time during the download procedure an error
			is encountered, the "onError" event is triggered, and
			the cache is -not- updated.</li>
		</ol>
		
		@param cacheDir The name of the directory to store files
		in. For instance "mycoolapp". This may not contain any
		further path information (i.e. no /'s or \'s), and will
		be stripped if provided.
		
		@param baseURL The url to use to fetch any media that
		may be required. This should be a fully qualified URL
		that contains protocol, host (if applicable), and possibly
		a relative directory path. E.g. "http://my.host.com/mycoolapp/media"
	*/
	constructor(cacheDir:String, baseURL:String) {}

	/**
		Test if a file is present in the local cache. If it is,
		then a true value is returned (<code>null</code> otherwise).
	*/	
	function exists(media:String):Boolean;
	
	/**
		Retrieve a media file. This performs a cache lookup
		and if necessary begins the download process (when it
		can) in order to obtain it. You must have previously
		added a listener to this cache object in order to know
		when the operation completes.
		<p>
		Use this even if you know the file has already been
		fetched previously as this will verify the integrity
		and "freshness" of the cached copy.
		
		@return This returns true if the file was immediately
			found to be either from a local file store or from
			a cached file that needed no further validation.
			Otherwise, it returns false to indicate the result
			will be provided at a later time via onComplete()
			or onError().
	*/
	function fetch(media:String);
	
	/**
		Erase any local copy of this media file from the cache.
	*/
	function flush(media:String);
	
	/**
		Erase all local copies of any files not currently being
		downloaded.
	*/
	function flushAll();
	
	/**
		Set the cache expiration time. Any file found in the cache
		with a modification time less than or equal to the specified
		number of seconds will assumed to be still valid and will
		not result in a remote source check. Note, even if the file
		is "stale", it may still not require a full download. The
		caching system will perform a conditional fetch from the
		remote source, and only re-download if necessary.
		<p>
		You generally want this to be a pretty large number (possibly
		on the order of several days or more). Setting this value too
		low results in frequent requests to the remote source in order
		to verify the file's integrity, causing undeserved load on
		the remote host if the file doesn't actually change often.
		Setting this value too high may cause new changes to the file
		to go unnoticed. This may be a problem if you're relying on
		the content of the file to be up to date.
		<p>
		The default value for this is 15 days.
		<p>
		To obtain a fresh copy regardless of a cache'd file's date
		you can either:
		<ol>
		<li>Set this value temporarily to 0, perform a fetch, then
		return it to a reasonable value.</li>
		<li>Or, perform a flush() prior to a corresponding fetch().</li>
		</ol>
		Both of the above options should give you a valid and
		up to date file, with the former being the "better" 
		alternative if at all possible (since it still may not involve
		a download if the contents have not changed). The latter
		forces a download regardless of whether the file is up to
		date or not.
		
		@param seconds The number of seconds that a file can be
			in the cache before being considered "stale".
	*/
	function setStalenessTime(seconds:Integer);
	
	/**
		Get the Base URL (the location which all relative URLs are 
		resolved from) that this media cache uses. The base URL
		is set via the constructor. Note there is no way to change
		a base URL once the cache has been created.
	*/
	function getBaseURL():String;
	
	
	/**
		Set a "cookie" in the cache. This is intended to be used
		as a simple persistent storage for arbitrary strings. It
		is not intended to be used for large scale storage (as
		memory constraints make it unweildy), but should work
		for many tasks. There is no hard limit on the size of the
		string you can store, but very large strings may cause
		perceptible lag when writing to disk.
		<p>
		To delete a cookie from the cache, simply set it to an
		empty string (or <tt>null</tt>).
		<p>
		To retrieve the cookie at a later time (even in another
		play session), use {@link #getCookie()}.
		<p>
		Use this along with System.encodeVars()/decodeVars() to
		conveniently store tabular data.
		
		@param name The name of the cookie to use when storing.
		@param value The value to set the cookie to (empty string
			or null will erase the cookie).
	*/
	function setCookie(name:String, value:String);
	
	/**
		Fetch a cookie value from the persistent store. If the
		cookie has been set, this will return the previous value,
		otherwise it returns "".
	*/
	function getCookie(name:String):String;
}

/**
	Create a texture using one or more "operations" on it in
	sequence. This class cannot be instantiated directly, you must use
	Composition.createProceduralTexture() to declare it.
*/
class ProceduralTexture
{
	/**
		Get the name of this procedural texture (as set when calling
		createProceduralTexture()).
	*/
	function getName():String;
	
	/**
		Delete all operations currently stored for this procedural
		texture.
	*/
	function reset();
	
	/**
		Force each procedural texture operation to be re-evaluated.
		This is not usually necessary for static textures (as this
		is done once automatically when the texture is first used).
		However, some operations require explicit updates (such as
		renderScene). This can be an expensive call depending on how
		the procedural texture is set up, so avoid calling excessively.
	*/
	function update();
	
	/**
		Alter regions of the texture by applying a hue shift,
		using a "tinting map" as a guide.
		For instance: given original texture A, and a tinting
		map T, you can set up regions of -A- that should receive a
		new color, but using -T- to define those regions.
		<p>
		The algorithm attempts to retain the saturation and
		brightness of the original color while mainly changing
		the hue so the new color most resembles the new color.
		<p>
		This process will also attempt to use a "colormap" to
		map symbolic region names to colors. A colormap is simply
		a text file with a special name and contents. For instance,
		for the tinting map "blah.png", the corresponding colormap
		should exist in the same directory and have the name 
		"blah.png.colormap". The contents could look something like
		this:
		<pre>
			# Color value in texture = symbolic color name
			343434=eyes
			234234=body
			003433=hair
		</pre>
		<p>
		If no colormap is found, you must use explicit colors in
		the "recoloring" table, not symbolic names.
		
		@param tintmap The texture map to use as the region-defining
			"guide" map during the colorization process.
		@param colors A table that maps colorizable regions
			(using either an alias or a region color) to new
			color values. For example (the following uses two
			colormap aliases and one color value as keys):
			<pre>
			{ eyes => "505050",
			  body => "230030",
			  "234234" => "989898" }
			</pre>
	*/
	function colorizeRegions(tintmap:String, colors:Table);
	
	/**
		Blend the specified texture using its alpha values.
	*/
	function blend(texture:String);
	
	/**
		Blend the regions of one or more source textures by using a guiding
		region color map. The guide defines what parts of the source
		image to blend (just like the colorize operation), but instead
		of changing the color, it copies it directly from the source
		textures.
		<p>
		Typical uses for this are to copy parts of a clothing texture
		over-top the skin/base texture for a model.
		<p>
		This also uses the colormap color aliasing described in
		{@link #colorizeRegions()} to allow the regions to be named
		using a logical name rather than a color hex value.
		<p>
		<b>NOTE: Currently all blended textures must be of the same size
		as the destination texture (and guide map)!</b>.
		
		@param regionMap The texture to use as the "guide" map
					when performing the blend operation(s). Each unique
					RGB triplet defines a region that can be blended
					independently.
		@param regions A table mapping regions to specified source
					textures. The keys should be either hex
					color values or colormap "names", and the values
					should be texture names. Any regions not listed
					in this table will remain untouched. For instance:
					<pre>
					{ gloves="OrnatePlate.png",
					  chest="RustyPlate.png",
					  boots="BootsOfDOOM.png" }
					</pre>
	*/
	function blendRegions(regionMap:String, regions:Table);
	
	
	/**
		Render the scene from the viewpoint of the camera. This will perform
		a render-to-texture using the given camera (and its projection information)
		and copy the contents to the procedural texture. You can alternately
		include the skies and/or overlays for this scene render. Also, a
		visibility mask can be used to selectively render only those objects
		with a matching visibility flags setting (see
		{@link MovableObject.setVisibilityFlags}).
		<p>
		<b>WARNING: Do not delete the camera given to this render operation (either
		explicitly or implicitly via scene node destruction). Doing so, will
		almost assuredly cause a crash!</b>
		<p>
		Also, do not "reuse" the default camera for this. It will most likely
		reset the aspect ratio and may potentially affect other rendering state
		associated with the camera.
	*/
	function renderScene(camera:Camera, renderSkies:Boolean,
			renderOverlays:Boolean, visibilityMask:Integer);
}

/**
   A random number generator matching up with Java's
*/
class Random
{
    /**
       Create a new random number generator. If no argument is provided,
       the seed will be based on the current time, otherwise the least
       significant 32 bits of a single integer parameter will be used.
       @param seed (Optional, Integer) The seed value to initialize with.
    */
    constructor(...);

    /**
       Set the seed to the specified integer

       @param seed lower 32 bits of seed to use (the upper 32 being zeroes)
    */
    function setSeed(seed:Integer);

    /**
       Get a pseudorandom integer

       @param max Optional maximum number -- the returned integer will
                  be uniformly between 0(inclusive) and
                  max(exclusive).  If unspecified will generate
                  between 0 and maxint.

       @return A pseudorandomly-generated integer
    */
    function nextInt(...):Integer;

    /**
       Get a pseudorandom boolean

       @return A pseudorandomly-generated boolean
    */
    function nextBool():Boolean;

    /**
       Get a pseudorandom float matching Java's nextFloat()
       <p>
       Note that due to floating point implementation differences this
       may be off a hair from the value Java would give, but will be
       within a rounding error worth and will leave the RNG precisely
       the way Java would -- i.e. the first float you get will be just
       as far off as the millionth.

       @return A pseudorandomly-generated float between 0.0 and 1.0
    */
    function nextFloat():Float;

    /**
       Get a pseudorandom float matching Java's nextDouble()
       <p>
       Note that due to floating point implementation differences this
       may be off a hair from the value Java would give, but will be
       within a rounding error worth and will leave the RNG precisely
       the way Java would -- i.e. the first float you get will be just
       as far off as the millionth.
       <p>
       Note also that this doesn't have significantly more precision
       than nextFloat() once the return value gets to squirrel-land,
       but nextFloat() and nextDouble() use the RNG differently so if
       you want to keep two in sync (perhaps a server and client
       version) you must use the same calls to both.

       @return A pseudorandomly-generated float between 0.0 and 1.0
    */
    function nextDouble():Float;
}


class System
{
	/**
		Encode a table of string variables into a single 
		serializeable string. This encodes the members of
		the given table using a x-www-form-encoded format
		This is the same format used for the query part (the
		part after the '?') of a HTTP URL (e.g.
		"http://foo.com?blah=1&a=123"). You can undo this
		process using decodeVars().
		<p>
		Each field of the table will be encoded with its
		corresponding value stringified (via _tostring()).
		<p>
		Please note that the decodeVars(encodeVars(table))
		process can be non-transitive if you use non-string
		variables in the original table. Since the decode step
		has no way of knowing what the original type was, it
		simply decodes it as a string (rather than, say, an
		integer).
		
		@param vars A table containing string(able) variables.
		
		@return The encoded string.
	*/
	static function encodeVars(vars:Table):String;
	
	/**
		Decode a x-www-form-encoded set of name=value pairs
		into strings in a table. If the destination table
		is specified, it will use that as a destination,
		otherwise a new table is created.
		<p>
		When decoding, it will create slots in the table as
		needed and override any values that may already be
		existing for those slots if not. Slots in the table
		that are not in the decoded string are left untouched.
		
		@param vars The encoded variables string.
		
		@param table (Optional) If present, this table will
			recieve any decoded variable values. Omitting
			this is the same as calling 
			<code>decodeVars(var, {})</code>.
			
		@return The table that recieved the decoded values.
	*/
	static function decodeVars(vars:String, ...):Table;
	
	/**
		Close down the player and release all resources. The actual
		behavior of this routine depends on what is actually running
		the player. If it's a standalone executable, it will likely
		terminate the application. If it's a web plugin, it may simply
		make the screen black. This function does not return.
	*/
	static function exit();


	/**
		Obtain an instantaneous snapshot of the internal profiling
		metrics for the various parts of the player framework. This
		is a very rough estimation of proportional time spent by
		each section of the code.
		<p>
		It returns a table with the keys being the name of the section
		and values being the proportional amount of time spent in that
		section (i.e. 0.0 to 1.0). 
		<p>
		The values returned are estimates only, and will likely often
		not add up to "100%". This variance is due to a lot of factors,
		including CPU type and background processes. It should only be
		used to identify relative hotspots and problem areas in particular
		scenes.
	*/
	static function profileSnapshot():Table;
	
	
	/**
		Perform a search of the currently defined resource path(s) and
		return any files matching the given filename pattern. The pattern
		is a standard "wildcard" pattern (e.g. "foo*.jpg").
		<p>
		Currently, actually opening or manipulating these files is not
		possible because they may be in a prepackaged archive that cannot
		be manipulated directly (in which case the manifest is scanned
		rather than a directory). This may change in the future with an
		API for reading entries from arbitrary locations, but for now, this
		is purely informational.
	*/
	static function findFiles(pattern:String):Array;
	
	
	/**
		Query the runtime for various statistics and metrics for the
		currently running system. This will return a table with the
		following:
		<dl>
		<dt>lastFPS</dt>
		<dd>Last frame's estimated FPS</dd>
		<dt>avgFPS</dt>
		<dd>A running average of the FPS</dd>
		<dt>bestFPS</dt>
		<dd>The best FPS since the last stats reset.</dd>
		<dt>worstFPS</dt>
		<dd>The worst FPS since the last stats reset.</dd>
		<dt>bestFrameTime</dt>
		<dd>The time (in milliseconds) for the best rendered frame
				since last reset.</dd>
		<dt>worstFrameTime</dt>
		<dd>The time (in milliseconds) for the worst rendered frame
				since last reset.</dd>
		<dt>textureMemoryUsage</dt>
		<dd>The number of bytes currently held by loaded textures
		    (RAM, not video memory)</dd>
		</dl>
	*/
	static function getStats():Table;


    /**
       Write the contents of a texture (or procedural texture) to a
       file on the local file system.  The path for the file name must
       already exist or this will fail.

       @param textureName The name of the texture or procedural
                          texture to write to disk

       @param filename The full filename, including path, to write the
                       contents to.  The file type is determined by
                       the extension on the filename.  Default
                       settings are used when writing to the file.

       @return True if the file was successfully written, false otherwise
     */
    static function writeTextureToFile( textureName:string, filename:string ):Boolean;

	/**
		Copy the given text string onto the system clipboard.
	*/
	static function setClipboard(value:String);
	
	/**
		Get the current clipboard contents as a string. If there is
		no valid content on the clipboard (or it is in an unrecognizable
		format), this returns an empty string (i.e. "").
	*/
	static function getClipboard():String;
}


/**
	Load a required "library" script if it hasn't been loaded already.
	Putting this at the top of a script creates an explicit dependency
	on another script so that the other will have been loaded prior to
	continuing, but only if it has not been loaded once already.
	<p>
	This uses a special naming convention to identify scripts. Notably,
	file extensions are -not- specified here. Also, directories must
	be explicitly listed from the 'root' script space (i.e. require
	resolves paths relative not to the script that contains the require
	call, but to the very first script that was executed).
	<p>
	So, let's say you had two library files with common classes and/or
	functions in files organized like so:
	<pre>
+ (root)
|- root.nut
|-+ libs
  |- foo.nut
  |- blah.nut
	</pre>
	When root.nut is executed, it could call <tt>require("libs/foo")</tt>
	and <tt>require("libs/blah")</tt> and those two .nut files will be
	found, loaded, compiled if necessary and executed before continuing
	within the original root.nut.
	<p>
	The file extensions are not specified so that precompiled script
	files (or in fact precompiled native libraries) can be used in the
	future without changing the underlying using code.
*/
function require(name:String);

/**
	This operates just like {@link require()} except that the script will
	be loaded and executed even if it has been already. If the script is
	not designed for this, it may cause problems. Usually the desired
	behavior is captured by the <tt>require()</tt> function, not this one.
*/
function include(name:String);

/**
	Serialize a value into a string representation of it. This will 
	traverse the structure recursively if any tables or arrays are
	found, otherwise it outputs a scalar value. To "undo" this process
	and get the original values back, call {@link unserialize()}.
*/
function serialize(value):String;

/**
	Take a previously {@link serialize()}'d value and reconstruct the
	data values encoded in the serialized string. If there is any
	problem decoding the values (badly formatted for instance), then
	it will throw an error.
	<p>
	Note, that this creates copies of the original data passed in, as
	it has no way of reconnecting to previously existing instances
	of each (they may have been saved ages ago!).
	
	@param serializedValue A string previously encoded with <tt>serialize()</tt>.
	
	@return The reconstructed object(s). 
*/
function unserialize(serializedValue);


/**
	Compile and execute a snippet of code. This will perform the same 
	logic as if the source code was loaded from a file and then interpreted.
	You may alternately provide a custom root table (instead of invoking
	within the current context).
	<p>
	When a root table is provided, this may be used to simulate a simple embedded
	scripting language (within scripts) (sometimes called Domain Specific Language).
	For example, to provide a very basic language with just sine and cosine functions
	provided, you could do the following:
	<pre>
	local myRoot = {
		cos = cos,
		sin = sin,
		print = print
	};
	eval("print(\"cos(1.5)\")", myRoot);
	</pre>
	<p>
	By providing this custom root table, you isolate a malicious or otherwise
	untrusted script from potentially reading or (worse!) writing to the root
	table you're using for normal scripting. If this behavior is not required
	(or you want to explicitly allow such behavior), simply omit the second
	parameter, and it will behave exactly as if the file was interpreted (with
	full access to the current root table).
	
	@param source A Squirrel source snippet
	@param rootTable (Optional) If provided, this will be used as the root table
		for the evaluated code. This is highly recommended.
	@return This will return the value (if any) returned by the evaluated source.
		Note that in order to get a value to return, you must actually have a
		return statement. E.g. eval("return 1") will return 1 as you'd expect,
		but eval("1") will return null as you might not.
*/
function eval(source, ...);


/**
	This is the global composition. A composition can use this to
	reference the "hosting" composition from any of its constituent
	scripts.
	<p>
	The root composition is special in the following ways:
	<ol>
	<li>There is only ever one root composition. The _root variable is
		read-only.</li>
	<li>The root composition receives "toplevel" input events. If a
		more specific UI_Element has not consumed an input event,
		it will eventually travel up the UI heirarchy and "land"
		on the root composition for processing.</li>
	<li>The root composition (or more specifically the codebase used
		when the root composition is loaded) determines where future
		relative-path media requests resolve to. For instance, a root
		composition of http://abc.com/foo.car generally has a code
		base of "http://abc.com/" and so a media cache request (via
		{@link _cache}) of "foo.txt" would look for
		"http://abc.com/foo.txt" when fetching the resource.</li>
	</ol>
*/
_root <- Composition();

/**
	The global scene. There is only ever one scene, and its contents can
	be added to and manipulated, but you cannot create or destroy the 
	scene itself. The scene is cleared when a root composition is set
	by the system.
*/
_scene <- Scene();

/**
	This is always set to the active camera being used to render the current
	scene. It is currently read-only (support for changing the active camera
	is planned however). Note, the camera may or may not be attached to a
	scene node (but if it isn't, it must be attached in order to move or
	orient it).
*/
_camera <- Camera();

/**
	The standard media cache used by the player to fetch remote resources.
	This is automatically set to point to the root composition's code
	base when the root composition is loaded. Thus, media requests from
	this cache are relative to the code base of the root component. Unless
	you have a specific need for a separate, isolated, cache, you should
	not need to instantiate your own cache object.
*/
_cache <- MediaCache();

/**
	The current time in milliseconds since the player was started.
*/
_time <- 0;

/**
	The number of milliseconds between the previous frame and this
	frame in the behavior/render loop.
*/
_deltat <- 0;

