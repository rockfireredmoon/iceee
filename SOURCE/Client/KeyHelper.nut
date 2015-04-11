class this.KeyHelper 
{
	static function getKeyText( evt, isPressEvent )
	{
		if (!(evt instanceof this.KeyEvent))
		{
			throw this.Exception("is not a KeyEvent");
		}

		return this.KeyHelper.keyBindText(evt.keyCode, evt.isControlDown(), evt.isAltDown(), evt.isShiftDown(), isPressEvent);
	}

	static function keyBindText( keyCode, ctrlDown, altDown, shiftDown, isPressEvent )
	{
		local str = "";

		if (ctrlDown && keyCode != this.Key.VK_CONTROL)
		{
			str += "Ctrl";
		}

		if (altDown && keyCode != this.Key.VK_ALT)
		{
			if (str.len() > 0)
			{
				str += "+";
			}

			str += "Alt";
		}

		if (shiftDown && keyCode != this.Key.VK_SHIFT)
		{
			if (str.len() > 0)
			{
				str += "+";
			}

			str += "Shift";
		}

		if (str.len() > 0)
		{
			str += "+";
		}

		str += this.Key.getText(keyCode);
		return isPressEvent ? str : "^" + str;
	}

	static function toChar( keyCode, shiftDown )
	{
		local keyString = "";

		if (keyCode >= 65 && keyCode <= 90)
		{
			keyString = keyCode.tochar();

			if (!shiftDown)
			{
				keyString = keyString.tolower();
			}

			return keyString;
		}

		switch(keyCode)
		{
		case 32:
			keyString = " ";
			break;

		case 48:
			if (shiftDown)
			{
				keyString = ")";
			}
			else
			{
				keyString = "0";
			}

			break;

		case 49:
			if (shiftDown)
			{
				keyString = "!";
			}
			else
			{
				keyString = "1";
			}

			break;

		case 50:
			if (shiftDown)
			{
				keyString = "@";
			}
			else
			{
				keyString = "2";
			}

			break;

		case 51:
			if (shiftDown)
			{
				keyString = "#";
			}
			else
			{
				keyString = "3";
			}

			break;

		case 52:
			if (shiftDown)
			{
				keyString = "$";
			}
			else
			{
				keyString = "4";
			}

			break;

		case 53:
			if (shiftDown)
			{
				keyString = "%";
			}
			else
			{
				keyString = "5";
			}

			break;

		case 54:
			if (shiftDown)
			{
				keyString = "^";
			}
			else
			{
				keyString = "6";
			}

			break;

		case 55:
			if (shiftDown)
			{
				keyString = "&";
			}
			else
			{
				keyString = "7";
			}

			break;

		case 56:
			if (shiftDown)
			{
				keyString = "*";
			}
			else
			{
				keyString = "8";
			}

			break;

		case 57:
			if (shiftDown)
			{
				keyString = "(";
			}
			else
			{
				keyString = "9";
			}

			break;

		case 96:
			keyString = "0";
			break;

		case 97:
			keyString = "1";
			break;

		case 98:
			keyString = "2";
			break;

		case 99:
			keyString = "3";
			break;

		case 100:
			keyString = "4";
			break;

		case 101:
			keyString = "5";
			break;

		case 102:
			keyString = "6";
			break;

		case 103:
			keyString = "7";
			break;

		case 104:
			keyString = "8";
			break;

		case 105:
			keyString = "9";
			break;

		case 106:
			keyString = "*";
			break;

		case 107:
			keyString = "+";
			break;

		case 109:
			keyString = "-";
			break;

		case 110:
			keyString = ".";
			break;

		case 111:
			keyString = "/";
			break;

		case 186:
			if (shiftDown)
			{
				keyString = ":";
			}
			else
			{
				keyString = ";";
			}

			break;

		case 187:
			if (shiftDown)
			{
				keyString = "+";
			}
			else
			{
				keyString = "=";
			}

			break;

		case 188:
			if (shiftDown)
			{
				keyString = "<";
			}
			else
			{
				keyString = ",";
			}

			break;

		case 189:
			if (shiftDown)
			{
				keyString = "_";
			}
			else
			{
				keyString = "-";
			}

			break;

		case 190:
			if (shiftDown)
			{
				keyString = ">";
			}
			else
			{
				keyString = ".";
			}

			break;

		case 191:
			if (shiftDown)
			{
				keyString = "?";
			}
			else
			{
				keyString = "/";
			}

			break;

		case 192:
			if (shiftDown)
			{
				keyString = "~";
			}
			else
			{
				keyString = "`";
			}

			break;

		case 219:
			if (shiftDown)
			{
				keyString = "{";
			}
			else
			{
				keyString = "[";
			}

			break;

		case 220:
			if (shiftDown)
			{
				keyString = "|";
			}
			else
			{
				keyString = "\\";
			}

			break;

		case 221:
			if (shiftDown)
			{
				keyString = "}";
			}
			else
			{
				keyString = "]";
			}

			break;

		case 222:
			if (shiftDown)
			{
				keyString = "\"";
			}
			else
			{
				keyString = "\'";
			}

			break;
		}

		return keyString;
	}

}

