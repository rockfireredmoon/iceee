this.Math <- {
	PI = 3.1415925,
	TWO_PI = this.PI * 2.0,
	HALF_PI = this.PI / 2.0,
	function max( a, b )
	{
		if (a > b)
		{
			return a;
		}

		return b;
	}

	function min( a, b )
	{
		if (a < b)
		{
			return a;
		}

		return b;
	}

	function abs( n )
	{
		if (n < 0)
		{
			return n * -1;
		}

		return n;
	}

	function manhattanDistanceXZ( a, b )
	{
		return this.Math.abs(a.x - b.x) + this.Math.abs(a.z - b.z);
	}

	function fuzzyCompare( v1, v2, epsilon )
	{
		if (this.abs(v1 - v2) <= epsilon)
		{
			return true;
		}

		return false;
	}

	function clamp( x, min, max )
	{
		if (x < min)
		{
			return min;
		}

		if (x > max)
		{
			return max;
		}

		return x;
	}

	function lerp( a, b, t )
	{
		return b * t + (1.0 - t) * a;
	}

	function slerpVectors( from, to, t )
	{
		local rot = to.getRotationTo(from, this.Vector3());
		local interpolatedRot = rot.slerp(t, this.Quaternion());
		local interpolatedVector = interpolatedRot.rotate(to);
		return interpolatedVector;
	}

	function polarAngle( x, y )
	{
		return this.atan2(x, y);
	}

	function rad2deg( rads )
	{
		return rads * 180.0 / this.PI;
	}

	function deg2rad( deg )
	{
		return deg * this.PI / 180.0;
	}

	function rad2byterot( rads )
	{
		if (rads < 0)
		{
			rads += this.TWO_PI;
		}

		return (rads / this.TWO_PI * 255).tointeger();
	}

	function byterot2rad( rot )
	{
		return rot / 255.0 * this.TWO_PI;
	}

	function lerpColor( a, b, t )
	{
		return this.Color(this.lerp(a.r, b.r, t), this.lerp(a.g, b.g, t), this.lerp(a.b, b.b, t), this.lerp(a.a, b.b, t));
	}

	function RoundFloat( pFloat, pDecimal )
	{
		local decMultiplier = this.pow(10, pDecimal);
		local newFloat = pFloat * decMultiplier.tofloat();
		local tempInt = newFloat.tointeger();
		newFloat = tempInt.tofloat() / decMultiplier.tofloat();
		return newFloat;
	}

	function DetermineRadAngleBetweenTwoPoints( pSourceVector, pTargetVector )
	{
		if (pTargetVector != pSourceVector)
		{
			local lVector = pTargetVector - pSourceVector;
			return this.atan2(lVector.x, lVector.z);
		}
		else
		{
			return 0;
		}
	}

	function DetermineDistanceBetweenTwoPoints( pSourceVector, pTargetVector )
	{
		return pTargetVector.distance(pSourceVector);
	}

	function ConvertVectorToRad( pTargetVector )
	{
		local pSourceVector = this.Vector3(0.0, 0.0, 0.0);
		return ::Math.DetermineRadAngleBetweenTwoPoints(pSourceVector, pTargetVector);
	}

	function GravitateValue( var, desired, delta, adjust )
	{
		if (var < desired)
		{
			var += (desired - var) * adjust * delta;

			if (var > desired)
			{
				var = desired;
			}
		}
		else if (var > desired)
		{
			var -= (var - desired) * adjust * delta;

			if (var < desired)
			{
				var = desired;
			}
		}

		return var;
	}

	function _ShortestRotationIsInverted( value1, value2 )
	{
		local directRoute = this.Math.abs(value1 - value2);
		local indirectRouteA = this.Math.abs(value1 - 360) + this.Math.abs(value2);
		local indirectRouteB = this.Math.abs(value1) + this.Math.abs(value2 - 360);
		local indirectRoute = indirectRouteA < indirectRouteB ? indirectRouteA : indirectRouteB;
		return directRoute < indirectRoute ? false : true;
	}

	function FloatModulos( value, base )
	{
		if (value >= base)
		{
			value -= (value / base).tointeger() * base;
		}

		if (value < 0)
		{
			value += ((-value / base).tointeger() + 1) * base;
		}

		return value;
	}

	function ShortestRotationDistance( value1, value2 )
	{
		local directRoute = this.Math.abs(value1 - value2);
		local indirectRouteA = this.Math.abs(value1 - 360) + this.Math.abs(value2);
		local indirectRouteB = this.Math.abs(value1) + this.Math.abs(value2 - 360);
		local indirectRoute = indirectRouteA < indirectRouteB ? indirectRouteA : indirectRouteB;
		return directRoute < indirectRoute ? directRoute : indirectRoute;
	}

	function GravitateAngle( var, desired, delta, adjust )
	{
		if (var == desired)
		{
			return var;
		}

		local shortestDist = this.Math.ShortestRotationDistance(desired, var);

		if (this.Math._ShortestRotationIsInverted(desired, var))
		{
			if (var < desired)
			{
				var -= shortestDist * adjust * delta;
			}
			else if (var > desired)
			{
				var += shortestDist * adjust * delta;
			}
		}
		else if (var < desired)
		{
			var += shortestDist * adjust * delta;

			if (var > desired)
			{
				var = desired;
			}
		}
		else if (var > desired)
		{
			var -= shortestDist * adjust * delta;

			if (var < desired)
			{
				var = desired;
			}
		}

		return this.Math.FloatModulos(var, 360.0);
	}

	function ConvertRadToVector( pRad )
	{
		local vector = this.Vector3(0.0, 0.0, 0.0);
		vector.z = this.cos(pRad);
		vector.x = this.sin(pRad);
		vector.y = 0;
		vector.normalize();
		return vector;
	}

	function ConvertPercentageToRad( pPercentage )
	{
		return pPercentage * this.TWO_PI;
	}

	function ConvertRadToFloatPercentage( pRad )
	{
		local percentage = pRad / this.TWO_PI;
		percentage = percentage % 1.0;

		if (percentage < 0)
		{
			percentage = percentage + 1.0;
		}

		return percentage;
	}

	function ConvertQuaternionToRad( pQuat )
	{
		local zAxis = pQuat.zAxis();
		return this.Math.ConvertVectorToRad(zAxis);
	}

	function ConvertRadToQuaternion( pRad )
	{
		local vector = this.Vector3(0.0, 1.0, 0.0);
		return this.Quaternion(pRad, vector);
	}

	function CalcuateShortestBetweenTwoRads( pSourceRad, pTargetRad )
	{
		local sourcePercentage = ::Math.ConvertRadToFloatPercentage(pSourceRad);
		local targetPercentage = ::Math.ConvertRadToFloatPercentage(pTargetRad);
		local firstChangePercentage = targetPercentage - sourcePercentage;
		local secondChangePercentage = 0.0;

		if (targetPercentage < sourcePercentage)
		{
			secondChangePercentage = targetPercentage + 1.0 - sourcePercentage;
		}
		else
		{
			secondChangePercentage = targetPercentage - 1.0 - sourcePercentage;
		}

		if (this.fabs(secondChangePercentage) < this.fabs(firstChangePercentage))
		{
			return this.ConvertPercentageToRad(secondChangePercentage);
		}
		else
		{
			return this.ConvertPercentageToRad(firstChangePercentage);
		}
	}

};
