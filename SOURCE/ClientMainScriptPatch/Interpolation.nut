/*
 * Implements various interpolation techniques. Borrowed from TonegodGUI  (Tonegod), which
 * in turn borrows from LibGdx (Nathan Sweet)
 *
 * Interpolators have no state and are singletones. May either be referenced directly
 * using the constants in the Interpolator table, or using Interpolator.interpolate(name)
 * where name is a lower-case string mnemonic for the helper.
 *
 * Custom interpolators of course can also be created by extended Interpolation.   
 */

class Interpolation {
	
	function doApply(a) {
		throw "Must implement doApply(a)";
	}
	
	function apply(start, end, a) {
		return start + ( end - start ) * doApply(a);
	}
}

class LinearInterpolation extends Interpolation {
	function doApply(a) {
		return a;
	}
}

class FadeInterpolation extends Interpolation {
	function doApply(a) {
		return Math.clamp(a * a * a * (a * (a * 6 - 15) + 10), 0, 1);
	}
}

class CircleInterpolation extends Interpolation {
	function doApply(a) {
		if(a <= 0.5) {
			a *= 2;
			return (1 - sqrt(1 - a * a)) / 2;
		}
		a--;
		a *= 2;
		return (sqrt(1 - a * a) + 1) / 2;
	}
}

class CircleInInterpolation extends Interpolation {
	function doApply(a) {
		return 1 - sqrt(1 - a * a);
	}
}

class CircleOutInterpolation extends Interpolation {
	function doApply(a) {
		a--;
		return sqrt(1 - a * a);
	}
}

class SineInterpolation extends Interpolation {
	function doApply(a) {
		return (1 - cos(a * Math.PI)) / 2;
	}
}

class SineInInterpolation extends Interpolation {
	function doApply(a) {
		return 1 - cos(a * Math.PI / 2);
	}
}

class SineOutInterpolation extends Interpolation {
	function doApply(a) {
		return 1 - cos(a * Math.PI / 2);
	}
}

class ExpInterpolation extends Interpolation {
	mValue = null;
	mPower = null;
	mMin = null;
	mScale = null;

	constructor(value, power) {
		mValue = value;
		mPower = power;
		mMin = pow(value, -power).tofloat();
		mScale = 1 / (1 - mMin);
	}

	function doApply(a) {
		if (a <= 0.5) 
			return (pow(mValue, mPower * (a * 2 - 1)) - mMin).tofloat() * mScale / 2;
		return (2 - (pow(mValue, -mPower * (a * 2 - 1)) - mMin).tofloat() * mScale) / 2;
	}
}

class ExpInInterpolation extends ExpInterpolation {

	constructor(value, power) {
		ExpInterpolation.constructor(value, power);
	}

	function doApply(a) {
		return (pow(mValue, mPower * (a - 1)).tofloat() - mMin) * mScale;
	}
}

class ExpOutInterpolation extends ExpInterpolation {

	constructor(value, power) {
		ExpInterpolation.constructor(value, power);
	}

	function doApply(a) {
		return 1 - (pow(mValue, -mPower * a).tofloat() - mMin) * mScale;
	}
}

class ElasticInterpolation extends Interpolation {

	mValue = null;
	mPowe = null;

	constructor(value, power) {
		mValue = value;
		mPower = power;
	}

	function doApply(a) {
			if (a <= 0.5) {
				a *= 2;
				return pow(mValue, mPower * (a - 1)).tofloat() * sin(a * 20) * 1.0955 / 2;
			}
			a = 1 - a;
			a *= 2;
			return 1.0 - pow(mValue, mPower * (a - 1)).tofloat() * sin((a) * 20) * 1.0955 / 2;
	}
}

class ElasticInInterpolation extends ElasticInterpolation {

	constructor(value, power) {
		ElasticInterpolation.constructor(value, power);
	}

	function doApply(a) {
		return pow(mValue, mPower * (a - 1)) * sin(a * 20) * 1.0955;
	}
}

class ElasticOutInterpolation extends ElasticInterpolation {

	constructor(value, power) {
		ElasticInterpolation.constructor(value, power);
	}

	function doApply(a) {
		a = 1 - a;
		return (1 - pow(mValue, mPower * (a - 1)).tofloat() * sin(a * 20) * 1.0955);
	}
}

class SwingInterpolation extends Interpolation {

	mScale = null;

	constructor(scale) {
		mScale = scale * 2;
	}

	function doApply(a) {
		if (a <= 0.5) {
			a *= 2;
			return a * a * ((mScale + 1) * a - mScale) / 2;
		}
		a--;
		a *= 2;
		return a * a * ((mScale + 1) * a + mScale) / 2 + 1;
	}
}

class SwingInInterpolation extends Interpolation {

	mScale = null;

	constructor(scale) {
		mScale = scale 
	}

	function doApply(a) {
		return a * a * ((mScale + 1) * a - mScale);
	}
}

class SwingOutInterpolation extends SwingInterpolation {

	constructor(scale) {
		mScale = scale 
	}

	function doApply(a) {
		a--;
		return a * a * ((mScale + 1) * a + mScale) + 1;
	}
}


class BounceOutInterpolation extends Interpolation {

	mWidths = null;
	mHeights = null;

	constructor(widths, heights) {
		if(widths.len() != heights.len())
			throw "Must be the same number of widths and heights";
		mWidths = widths;
		mHeights = heights; 
	}
	
	constructor(bounces) {
		if (bounces < 2 || bounces > 5) 
			throw "bounces cannot be < 2 or > 5: " + bounces;
		mWidths = array(bounces,0);
		mHeights = array(bounces,0);
		mHeights[0] = 1;
		switch (bounces) {
		case 2:
			mWidths[0] = 0.6;
			mWidths[1] = 0.4;
			mHeights[1] = 0.33;
			break;
		case 3:
			mWidths[0] = 0.4;
			mWidths[1] = 0.4;
			mWidths[2] = 0.2;
			mHeights[1] = 0.33;
			mHeights[2] = 0.1;
			break;
		case 4:
			mWidths[0] = 0.34;
			mWidths[1] = 0.34;
			mWidths[2] = 0.2;
			mWidths[3] = 0.15;
			mHeights[1] = 0.26;
			mHeights[2] = 0.11;
			mHeights[3] = 0.03;
			break;
		case 5:
			mWidths[0] = 0.3;
			mWidths[1] = 0.3;
			mWidths[2] = 0.2;
			mWidths[3] = 0.1;
			mWidths[4] = 0.1;
			mHeights[1] = 0.45;
			mHeights[2] = 0.3;
			mHeights[3] = 0.15;
			mHeights[4] = 0.06;
			break;
		}
		mWidths[0] *= 2;
	}

	function doApply(a) {
		a += mWidths[0] / 2;
		local width = 0.0;
		local height = 0.0;
		for (local i = 0, n = mWidths.len(); i < n; i++) {
			width = mWidths[i];
			if (a <= width) {
				height = mHeights[i];
				break;
			}
			a -= width;
		}
		a /= width;
		local z = 4 / width * height * a;
		return 1 - (z - z * a) * width;
	}
}

class BounceInterpolation extends BounceOutInterpolation {
	constructor(widths, heights) {
		BounceOutInterpolation.constructor(widths,heights); 
	}
	
	constructor(bounces) {
		BounceOutInterpolation.constructor(bounces);
	}
	
	function out(a) {
		local test = a + mWidths[0] / 2;
		if(test < mWidths[0])
			return test / (mWidths[0] / 2) - 1;
		return BounceOutInterpolation.doApply(a);
	}

	function doApply(a) {
		if (a <= 0.5) return (1 - out(1 - a * 2)) / 2;
		return out(a * 2 - 1) / 2 + 0.5;
	}
}

class BounceInInterpolation extends BounceOutInterpolation {
	constructor(widths, heights) {
		BounceOutInterpolation.constructor(widths,heights); 
	}
	
	constructor(bounces) {
		BounceOutInterpolation.constructor(bounces);
	}
	
	function doApply(a) {
		return 1 - BounceOutInterpolation.doApply(1 - a);
	}
}

class PowInterpolation extends Interpolation {
	mPower = null;

	constructor(power) {
		mPower = power; 
	}
	
	function doApply(a) {
		if (a <= 0.5) 
			return pow(a * 2, mPower) / 2;
		return pow((a - 1) * 2, mPower).tofloat() / (mPower % 2 == 0 ? -2 : 2) + 1;
	}
}

class PowInInterpolation extends PowInterpolation {

	constructor(power) {
		PowInterpolation.constructor(power); 
	}
	
	function doApply(a) {
		return pow(a, mPower).tofloat();
	}
}

class PowOutInterpolation extends PowInterpolation {

	constructor(power) {
		PowInterpolation.constructor(power); 
	}
	
	function doApply(a) {
		return pow(a - 1, mPower).tofloat() * (mPower % 2 == 0 ? -1 : 1) + 1;
	}
}

Interpolator <- {};
Interpolator.INTERPOLATION_LINEAR <- LinearInterpolation();
Interpolator.INTERPOLATION_FADE <- LinearInterpolation();
Interpolator.INTERPOLATION_CIRCLE <- CircleInterpolation();
Interpolator.INTERPOLATION_CIRCLE_IN <- CircleInInterpolation();
Interpolator.INTERPOLATION_CIRCLE_OUT <- CircleOutInterpolation();
Interpolator.INTERPOLATION_SINE <- SineInterpolation();
Interpolator.INTERPOLATION_SINE_IN <- SineInInterpolation();
Interpolator.INTERPOLATION_SINE_OUT <- SineInInterpolation();
Interpolator.INTERPOLATION_EXP10 <- ExpInterpolation(2.0, 10..0);
Interpolator.INTERPOLATION_EXP10_IN <- ExpInInterpolation(2.0, 10.0);
Interpolator.INTERPOLATION_EXP10_OUT <- ExpOutInterpolation(2.0, 10.0);
Interpolator.INTERPOLATION_EXP5 <- ExpInterpolation(2.0, 5.0);
Interpolator.INTERPOLATION_EXP5_IN <- ExpInInterpolation(2.0, 5.0);
Interpolator.INTERPOLATION_EXP5_OUT <- ExpOutInterpolation(2.0, 5.0);
Interpolator.INTERPOLATION_ELASTIC <- ExpInterpolation(2.0, 10.0);
Interpolator.INTERPOLATION_ELASTIC_IN <- ExpInInterpolation(2.0, 10.0);
Interpolator.INTERPOLATION_ELASTIC_OUT <- ExpOutInterpolation(2.0, 10.0);
Interpolator.INTERPOLATION_SWING <- SwingInterpolation(1.5);
Interpolator.INTERPOLATION_SWING_IN <- SwingInInterpolation(2.0);
Interpolator.INTERPOLATION_SWING_OUT <- SwingOutInterpolation(2.0);
Interpolator.INTERPOLATION_BOUNCE <- BounceInterpolation(4);
Interpolator.INTERPOLATION_BOUNCE_IN <- BounceInInterpolation(4);
Interpolator.INTERPOLATION_BOUNCE_OUT <- BounceOutInterpolation(4);
Interpolator.INTERPOLATION_POW2 <- PowInterpolation(2);
Interpolator.INTERPOLATION_POW2_IN <- PowInInterpolation(2);
Interpolator.INTERPOLATION_POW2_OUT <- PowOutInterpolation(2);
Interpolator.INTERPOLATION_POW3 <- PowInterpolation(3);
Interpolator.INTERPOLATION_POW3_IN <- PowInInterpolation(3);
Interpolator.INTERPOLATION_POW3_OUT <- PowOutInterpolation(3);
Interpolator.INTERPOLATION_POW4 <- PowInterpolation(4);
Interpolator.INTERPOLATION_POW4_IN <- PowInInterpolation(4);
Interpolator.INTERPOLATION_POW4_OUT <- PowOutInterpolation(4);
Interpolator.INTERPOLATION_POW5 <- PowInterpolation(5);
Interpolator.INTERPOLATION_POW5_IN <- PowInInterpolation(5);
Interpolator.INTERPOLATION_POW5_OUT <- PowOutInterpolation(5);
Interpolator.interpolate <- function ( name ) {
	switch(name) {
	case "fade" :
		return INTERPOLATION_FADE;
	case "circle" :
		return INTERPOLATION_CIRCLE;
	case "circlein" :
		return INTERPOLATION_CIRCLE_IN;
	case "circleout" :
		return INTERPOLATION_CIRCLE_OUT;
	case "sine" :
		return INTERPOLATION_SINE;
	case "sinein" :
		return INTERPOLATION_SINE_IN;
	case "sineout" :
		return INTERPOLATION_SINE_OUT;
	case "exp10" :
		return INTERPOLATION_EXP10;
	case "exp10in" :
		return INTERPOLATION_EXP10_IN;
	case "exp10out" :
		return INTERPOLATION_EXP10_OUT;
	case "exp5" :
		return INTERPOLATION_EXP5;
	case "exp5in" :
		return INTERPOLATION_EXP5_IN;
	case "exp5out" :
		return INTERPOLATION_EXP5_OUT;
	case "elastic" :
		return INTERPOLATION_ELASTIC;
	case "elasticin" :
		return INTERPOLATION_ELASTIC_IN;
	case "elasticout" :
		return INTERPOLATION_ELASTIC_OUT;
	case "swing" :
		return INTERPOLATION_SWING;
	case "swingin" :
		return INTERPOLATION_SWING_IN;
	case "swingout" :
		return INTERPOLATION_SWING_OUT;
	case "bounce" :
		return INTERPOLATION_BOUNCE;
	case "bouncein" :
		return INTERPOLATION_BOUNCE_IN;
	case "bounceout" :
		return INTERPOLATION_BOUNCE_OUT;
	case "pow2" :
		return INTERPOLATION_POW2;
	case "pow2in" :
		return INTERPOLATION_POW2_IN;
	case "pow2out" :
		return INTERPOLATION_POW2_OUT;
	case "pow3" :
		return INTERPOLATION_POW3;
	case "pow3in" :
		return INTERPOLATION_POW3_IN;
	case "pow3out" :
		return INTERPOLATION_POW3_OUT;
	case "pow4" :
		return INTERPOLATION_POW4;
	case "pow4in" :
		return INTERPOLATION_POW4_IN;
	case "pow4out" :
		return INTERPOLATION_POW4_OUT;
	case "pow5" :
		return INTERPOLATION_POW5;
	case "pow5in" :
		return INTERPOLATION_POW5_IN;
	case "pow5out" :
		return INTERPOLATION_POW5_OUT;
	case "linear" :
	default:
		return INTERPOLATION_LINEAR;
	}
}
