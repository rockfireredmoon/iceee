this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.AbEditScreen extends this.GUI.Frame
{
	static ABILITY = 0;
	static STAT_KIT = 1;
	static AI_PACKAGE = 2;
	static SPAWN_PACKAGE = 3;
	static SPAWN_TABLE = 4;
	static ITEM = 5;
	static BODY_TYPE = 6;
	static CREDIT_ITEM = 7;
	static SPECIAL_OFFER = 8;
	static mClassName = "Screens.AbEditScreen";
	static mAbilityHeader = "ID\t\tName\t\t\tPresentation";
	static mBodyTypeHeader = "BodyType\tSize";
	static mStatkitHeader = "Kit\tLevel\tSTRENGTH";
	static mAIPackageHeader = "Package Name\tContent";
	static mSpawnPackageHeader = "Package Name\tCycle\tMob Total";
	static mSpawnTableHeader = "Table Name\tWeight\tPoints";
	static mItemHeader = "Item Name\tDisplay Name\tIcon";
	static mCreditItemHeader = "\t\t\t\tCredit Items";
	static mSpecialOfferHeader = "\t\t\tSpecial Offers";
	mEditingType = 0;
	mImportButton = null;
	mOutput = null;
	mTimer = null;
	mStatusLabel = null;
	mImportIndex = null;
	mImportName = null;
	mImportDefLines = null;
	mImportErrors = 0;
	mImportChanges = null;
	mCurrentSpawnPackage = null;
	mCurrentSpawnTable = null;
	constructor()
	{
		this.GUI.Frame.constructor("Import Screen");
		this.mImportButton = this._createButton("Import", "onImport");
		this.mStatusLabel = this.GUI.Label("Import settings from clipboard to begin.");
		this.mOutput = this.GUI.HTML();
		local cmain = this.GUI.Container(this.GUI.BorderLayout());
		cmain.setInsets(5);
		cmain.add(this.mImportButton, this.GUI.BorderLayout.NORTH);
		cmain.add(this.GUI.ScrollPanel(this.mOutput), this.GUI.BorderLayout.CENTER);
		cmain.add(this.mStatusLabel, this.GUI.BorderLayout.SOUTH);
		this.setContentPane(cmain);
		this.setSize(600, 400);
		this.centerOnScreen();
		this.setCached(::Pref.get("video.UICache"));
	}

	function _createButton( label, msg )
	{
		local b = this.GUI.Button(label);
		b.setReleaseMessage(msg);
		b.addActionListener(this);
		return b;
	}

	function onImport( button )
	{
		if (this.mImportIndex != null)
		{
			this.mImportIndex = null;
			this.mImportButton.setText("Import");
			return;
		}

		local text = this.System.getClipboard();
		local lines = this.Util.splitQuoteSafe(text, "\n");
		local n;

		for( n = 0; n < lines.len(); n++ )
		{
			lines[n] = this.Util.replace(lines[n], "\r", "");
		}

		local parsedLines = lines;

		if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mAbilityHeader))
		{
			this.mEditingType = this.ABILITY;
			this.mImportIndex = 2;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mStatkitHeader))
		{
			this.mEditingType = this.STAT_KIT;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mAIPackageHeader))
		{
			this.mEditingType = this.AI_PACKAGE;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mSpawnPackageHeader))
		{
			this.mEditingType = this.SPAWN_PACKAGE;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mSpawnTableHeader))
		{
			this.mEditingType = this.SPAWN_TABLE;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mItemHeader))
		{
			this.mEditingType = this.ITEM;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mBodyTypeHeader))
		{
			this.mEditingType = this.BODY_TYPE;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mCreditItemHeader))
		{
			this.mEditingType = this.CREDIT_ITEM;
			this.mImportIndex = 0;
		}
		else if (parsedLines.len() > 0 && this.Util.startsWith(parsedLines[0], this.mSpecialOfferHeader))
		{
			this.mEditingType = this.SPECIAL_OFFER;
			this.mImportIndex = 0;
		}
		else
		{
			this.GUI.MessageBox.show("The clipboard does not appear to contain a valid table for any import table \"export\".");
			return;
		}

		this.mStatusLabel.setText("");
		this.mImportButton.setText("Cancel");
		this.mOutput.setText("");
		this.postOutput("Beginning import.");
		this.mImportDefLines = parsedLines;
		this.mImportErrors = 0;
		this.mImportChanges = [];
		this._importNext();
	}

	function _toMilliseconds( str )
	{
		return (secs * 1000).tointeger();
		  // [014]  OP_POPTRAP        1      0    0    0
		  // [015]  OP_JMP            0      2    0    0
		return 0;
	}

	function _importNextStatKit()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			local args = [];
			args.append(fields[0]);
			args.append(fields[1]);
			args.append("STRENGTH");
			args.append(fields[2]);
			args.append("DEXTERITY");
			args.append(fields[3]);
			args.append("CONSTITUTION");
			args.append(fields[4]);
			args.append("PSYCHE");
			args.append(fields[5]);
			args.append("SPIRIT");
			args.append(fields[6]);
			args.append("MELEE_ATTACK_SPEED");
			args.append(fields[7]);
			args.append("MAGIC_ATTACK_SPEED");
			args.append(fields[8]);
			args.append("BASE_DAMAGE_MELEE");
			args.append(fields[9]);
			args.append("BASE_DAMAGE_FIRE");
			args.append(fields[10]);
			args.append("BASE_DAMAGE_FROST");
			args.append(fields[11]);
			args.append("BASE_DAMAGE_MYSTIC");
			args.append(fields[12]);
			args.append("BASE_DAMAGE_DEATH");
			args.append(fields[13]);
			args.append("DAMAGE_RESIST_MELEE");
			args.append(fields[14]);
			args.append("DAMAGE_RESIST_FIRE");
			args.append(fields[15]);
			args.append("DAMAGE_RESIST_FROST");
			args.append(fields[16]);
			args.append("DAMAGE_RESIST_MYSTIC");
			args.append(fields[17]);
			args.append("DAMAGE_RESIST_DEATH");
			args.append(fields[18]);
			args.append("BASE_MOVEMENT");
			args.append(fields[19]);
			args.append("BASE_LUCK");
			args.append(fields[20]);
			args.append("BASE_HEALTH");
			args.append(fields[21]);
			args.append("WILL_MAX");
			args.append(fields[22]);
			args.append("WILL_REGEN");
			args.append(fields[23]);
			args.append("MIGHT_MAX");
			args.append(fields[24]);
			args.append("MIGHT_REGEN");
			args.append(fields[25]);
			args.append("BASE_DODGE");
			args.append(fields[26]);
			args.append("BASE_DEFLECT");
			args.append(fields[27]);
			args.append("BASE_PARRY");
			args.append(fields[28]);
			args.append("BASE_BLOCK");
			args.append(fields[29]);
			args.append("BASE_MELEE_TO_HIT");
			args.append(fields[30]);
			args.append("BASE_MAGIC_SUCCESS");
			args.append(fields[31]);
			args.append("BASE_MELEE_CRITICAL");
			args.append(fields[32]);
			args.append("BASE_MAGIC_CRITICAL");
			args.append(fields[33]);
			args.append("OFFHAND_WEAPON_DAMAGE");
			args.append(fields[34]);
			args.append("BASE_HEALING");
			args.append(fields[35]);
			args.append("CASTING_SETBACK_CHANCE");
			args.append(fields[36]);
			args.append("CHANNELING_BREAK_CHANCE");
			args.append(fields[37]);
			this.mStatusLabel.setText("Updating " + this.mImportIndex + " of " + (this.mImportDefLines.len() - 1) + " (" + fields[0] + ")...");
			this._Connection.sendQuery("statkit.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Import Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				::_Connection.sendQuery("updateContent", this, [
					"statkits"
				]);
				return;
			}

			return;
		}
	}

	function _importNextBodyType()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			local args = [];
			args.append(fields[0]);
			args.append(fields[1]);
			this.mStatusLabel.setText("Updating " + this.mImportIndex + " of " + (this.mImportDefLines.len() - 1) + " (" + fields[0] + ")...");
			this._Connection.sendQuery("bodyType.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Input Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function skipWithError( errorString )
	{
		this.mImportName = "Line " + (this.mImportIndex + 1);
		this.onQueryError(null, errorString);
	}

	function _importNextCreditItem()
	{
		this.mImportIndex++;

		if (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");

			if (fields[0] == "" || fields[0] == "Offer ID #")
			{
				this._importNext();
				return;
			}

			local args = [];
			args.append(fields[0]);

			if (fields[1] != "")
			{
				args.append("priceAmount");
				args.append(fields[1]);
			}
			else
			{
				this.skipWithError("Need to input a Cost");
				return;
			}

			local stackCount = fields[3] != "" ? fields[3] : "0";
			args.append("itemProto");
			args.append(fields[2] + ":" + stackCount);
			args.append("title");
			args.append(fields[4]);
			args.append("description");
			args.append(fields[5]);

			if (fields[6] != "")
			{
				args.append("category");
				args.append(fields[6]);
			}
			else
			{
				this.skipWithError("Need to input a Category field RECIPES, ARMOR, BAGS, CONSUMABLES, CHARMS");
				return;
			}

			args.append("beginDate");
			args.append(fields[7]);
			args.append("endDate");
			args.append(fields[8]);
			args.append("quantityLimit");
			args.append(fields[9] != "" ? fields[9] : "-1");

			if (fields[10] != "")
			{
				args.append("status");
				args.append(fields[10]);
			}
			else
			{
				this.skipWithError("Need to input a Status field NEW, ACTIVE, EXPIRED, HIDDEN, CANCELED, HOT");
				return;
			}

			this.mStatusLabel.setText("Updating " + this.mImportIndex + " of " + (this.mImportDefLines.len() - 1) + " (" + fields[0] + ")...");
			::_Connection.sendQuery("item.market.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Import Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				::_creditShopManager.requestItemMarketList();
				return;
			}

			return;
		}
	}

	function _importNextSpecialOffer()
	{
		this.mImportIndex++;

		if (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");

			if (fields[0] == "" || fields[0] == "Offer ID #")
			{
				this._importNext();
				return;
			}

			local args = [];
			args.append(fields[0]);

			if (fields[1] != "")
			{
				args.append("percentDiscount");
				args.append(fields[1]);
			}
			else
			{
				this.skipWithError("Need to input a discount");
				return;
			}

			if (fields[2] != "")
			{
				args.append("creditOfferId");
				args.append(fields[2]);
			}
			else
			{
				this.skipWithError("Need to input a the credit Item Id you want to be sold");
				return;
			}

			args.append("title");
			args.append(fields[3]);
			args.append("description");
			args.append(fields[4]);
			local triggerEvents = [
				"BuffExpireTrigger",
				"EnterLocationTrigger",
				"LevelUpTrigger",
				"AcceptQuestTrigger",
				"CompleteQuestTrigger",
				"InteractWithTrigger",
				"InventoryFullTrigger",
				"DestroyItemTrigger",
				"LoginTimeTrigger",
				"UseLastItemTrigger",
				"UseLastBuffTrigger",
				"KillCreatureTrigger",
				"AbilityUseTrigger",
				"PlayerDeathTrigger",
				"UseDifferentAbilityTrigger",
				"AbilityPointTrigger"
			];
			local foundTriggerName = false;

			foreach( triggerName in triggerEvents )
			{
				if (triggerName == fields[5])
				{
					foundTriggerName = true;
				}
			}

			if (foundTriggerName)
			{
				args.append("triggerEventName");
				args.append(fields[5]);
			}
			else
			{
				this.skipWithError("Need to input a Trigger Event Name BuffExpireTrigger, EnterLocationTrigger, LevelUpTrigger," + " AcceptQuestTrigger, CompleteQuestTrigger, InteractWithTrigger, InventoryFullTrigger, DestroyItemTrigger," + " LoginTimeTrigger, UseLastItemTrigger, KillCreatureTrigger, AbilityUseTrigger, PlayerDeathTrigger, " + "UseDifferentAbilityTrigger, AbilityPointTrigger");
				return;
			}

			if (fields[6] != "")
			{
				args.append("triggerType");
				args.append(fields[6]);
			}
			else
			{
				this.skipWithError("Need to input a the trigger Type, NUMBER_TIMES, SEQUENTIALLY, CHANCE");
				return;
			}

			args.append("triggerAmount");
			args.append(fields[7] != "" ? fields[7] : "0");

			if (fields[8] != "")
			{
				args.append("param");
				args.append(fields[8]);
			}
			else if (!(fields[5] == "DestroyItemTrigger" || fields[5] == "InventoryFullTrigger" || fields[5] == "KillCreatureTrigger"))
			{
				this.skipWithError("Please enter a parameter needed for this trigger, for" + " ex: LevelUpTrigger you would need to enter the level that you want this trigger to fire at");
				return;
			}
			else
			{
				args.append("param");
				args.append("");
			}

			if (fields[9] != "")
			{
				args.append("status");
				args.append(fields[9]);
			}
			else
			{
				this.skipWithError("Need to input a Status field ACTIVE, INACTIVE");
				return;
			}

			this.mStatusLabel.setText("Updating " + this.mImportIndex + " of " + (this.mImportDefLines.len() - 1) + " (" + fields[0] + ")...");
			::_Connection.sendQuery("special.offer.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Import Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function _importNextAIPackage()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			local args = [];
			args.append(fields[0]);
			local test = fields[1].slice(1, fields[1].len() - 1);
			args.append(test);
			this.mStatusLabel.setText("Updating " + this.mImportIndex + " of " + (this.mImportDefLines.len() - 1) + " (" + fields[0] + ")...");
			this._Connection.sendQuery("aipackage.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Import Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function _importNextWeapon()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			this.mImportName = fields[0];
			local args = [];
			args.append("IMPORT");
			args.append(this.mImportName);
			args.append("displayName");
			args.append(fields[1]);
			args.append("icon");
			args.append(fields[2]);
			args.append("useAbility");
			args.append(fields[3]);
			args.append("actionAbility");
			args.append(fields[4]);
			args.append("copperValue");
			args.append(fields[5]);
			args.append("equipType");
			args.append(fields[6]);
			args.append("equipEffect");
			args.append(fields[7]);
			args.append("weaponType");
			args.append(fields[8]);
			args.append("armorType");
			args.append(fields[9]);
			args.append("weaponDamageMinRating");
			args.append(fields[10]);
			args.append("weaponDamageMaxRating");
			args.append(fields[11]);
			args.append("weaponSpeedRating");
			args.append(fields[12]);
			args.append("weaponExtraDamageRating");
			args.append(fields[13]);
			args.append("weaponExtraDamageType");
			args.append(fields[14]);
			args.append("armorResistMeleeRating");
			args.append(fields[15]);
			args.append("armorResistFireRating");
			args.append(fields[16]);
			args.append("armorResistFrostRating");
			args.append(fields[17]);
			args.append("armorResistMysticRating");
			args.append(fields[18]);
			args.append("armorResistDeathRating");
			args.append(fields[19]);
			args.append("bonusStrengthRating");
			args.append(fields[20]);
			args.append("bonusDexterityRating");
			args.append(fields[21]);
			args.append("bonusConstitutionRating");
			args.append(fields[22]);
			args.append("bonusPsycheRating");
			args.append(fields[23]);
			args.append("bonusHealthRating");
			args.append(fields[24]);
			args.append("bonusWillRating");
			args.append(fields[25]);
			args.append("bonusSpiritRating");
			args.append(fields[26]);
			this._Connection.sendQuery("item.def.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Import Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function _importNextItem()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			this.mImportName = fields[0];
			local args = [];
			args.append("IMPORT");
			args.append(this.mImportName);
			args.append("displayName");
			args.append(fields[1]);
			args.append("icon");
			args.append(fields[2]);
			args.append("useAbility");
			args.append(fields[3]);
			args.append("actionAbility");
			args.append(fields[4]);
			args.append("copperValue");
			args.append(fields[5]);
			args.append("equipType");
			args.append(fields[6]);
			args.append("equipEffect");
			args.append(fields[7]);
			args.append("weaponType");
			args.append(fields[8]);
			args.append("armorType");
			args.append(fields[9]);
			args.append("weaponDamageMinRating");
			args.append(fields[10]);
			args.append("weaponDamageMaxRating");
			args.append(fields[11]);
			args.append("weaponSpeedRating");
			args.append(fields[12]);
			args.append("weaponExtraDamageRating");
			args.append(fields[13]);
			args.append("weaponExtraDamageType");
			args.append(fields[14]);
			args.append("armorResistMeleeRating");
			args.append(fields[15]);
			args.append("armorResistFireRating");
			args.append(fields[16]);
			args.append("armorResistFrostRating");
			args.append(fields[17]);
			args.append("armorResistMysticRating");
			args.append(fields[18]);
			args.append("armorResistDeathRating");
			args.append(fields[19]);
			args.append("bonusStrengthRating");
			args.append(fields[20]);
			args.append("bonusDexterityRating");
			args.append(fields[21]);
			args.append("bonusConstitutionRating");
			args.append(fields[22]);
			args.append("bonusPsycheRating");
			args.append(fields[23]);
			args.append("bonusHealthRating");
			args.append(fields[24]);
			args.append("bonusWillRating");
			args.append(fields[25]);
			args.append("bonusSpiritRating");
			args.append(fields[26]);
			this._Connection.sendQuery("item.def.edit", this, args);

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Input Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function postOutput( text, ... )
	{
		this.mOutput.setText(text + "<br/>" + this.mOutput.getText());
	}

	function postError( text )
	{
		this.mOutput.setText("<font color=\"ff0000\">" + text + "</font><br/>" + this.mOutput.getText());
	}

	function _importNextSpawnTable()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			local args = [];

			if (fields[0] != "")
			{
				this.mCurrentSpawnTable = fields[0];
				this.mImportName = this.mCurrentSpawnTable;
				args.append(this.mCurrentSpawnTable);
				this._Connection.sendQuery("spawn.table.edit", this, args);
			}
			else
			{
				if (this.mCurrentSpawnTable == null)
				{
					return;
				}

				this.mImportName = this.mCurrentSpawnTable + " Entry";
				args.append(this.mCurrentSpawnTable);
				args.append("weight");
				args.append(fields[1]);
				args.append("points");
				args.append(fields[2]);
				args.append("minSpawned");
				args.append(fields[3]);
				args.append("maxSpawned");
				args.append(fields[4]);
				args.append("creatureDef");
				args.append(fields[5]);
				this._Connection.sendQuery("spawn.table.entry.edit", this, args);
			}

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Input Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function _importNextSpawnPackage()
	{
		this.mImportIndex++;

		while (this.mImportIndex < this.mImportDefLines.len())
		{
			local fields = this.Util.split(this.mImportDefLines[this.mImportIndex], "\t");
			local args = [];

			if (fields[0] != "")
			{
				this.mCurrentSpawnPackage = fields[0];
				this.mImportName = this.mCurrentSpawnPackage;
				args.append(this.mCurrentSpawnPackage);
				args.append("cycle");
				args.append(fields[1]);
				args.append("mobTotal");
				args.append(fields[2]);
				this._Connection.sendQuery("spawn.package.edit", this, args);
			}
			else
			{
				if (this.mCurrentSpawnPackage == null)
				{
					return;
				}

				this.mImportName = this.mCurrentSpawnPackage + " Entry";
				args.append(this.mCurrentSpawnPackage);
				args.append("spawnTable");
				args.append(fields[3]);
				args.append("scoreForNext");
				args.append(fields[4]);
				args.append("decayAmount");
				args.append(fields[5]);
				args.append("decayTime");
				args.append(fields[6]);
				args.append("maxActive");
				args.append(fields[7]);
				args.append("respawnRate");
				args.append(fields[8]);
				args.append("shiftDelay");
				args.append(fields[9]);
				this._Connection.sendQuery("spawn.package.entry.edit", this, args);
			}

			if (this.mImportIndex >= this.mImportDefLines.len() - 1)
			{
				this.mImportIndex = null;
				this.postOutput("Import Complete");
				local str = "Import complete. (";
				str += this.mImportDefLines.len() - 1 + " modified";

				if (this.mImportErrors)
				{
					str += ", " + this.mImportErrors + " error(s)";
				}

				str += ".)";
				this.mStatusLabel.setText(str);
				this.mImportButton.setText("Import");
				return;
			}

			return;
		}
	}

	function _dressFidelity( fidelity, parts )
	{
		if (parts.cooldownCategory == "Resurrection")
		{
			return fidelity + " Dead";
		}

		if (fidelity == null || fidelity == "")
		{
			return fidelity;
		}

		if (parts.abilityId.tointeger() >= 10000)
		{
			return fidelity;
		}

		return fidelity + " Alive";
	}

	function compileAbilityQuery( parts )
	{
		local result = [];
		result.append(parts.abilityId);
		result.append("groupId");
		result.append(parts.groupId);
		local coordinateParts = this.Util.split(parts.slotCoordinate, ",");
		result.append("slotCoordinateX");
		result.append(coordinateParts.len() == 2 ? this.Util.trim(coordinateParts[0]) : "0");
		result.append("slotCoordinateY");
		result.append(coordinateParts.len() == 2 ? this.Util.trim(coordinateParts[1]) : "0");
		result.append("name");
		result.append(parts.name);
		result.append("duration");
		result.append(parts.duration != "" ? parts.duration : "0");
		result.append("iterationInterval");
		result.append(parts.interval != "" ? parts.interval : "0");
		result.append("description");
		result.append(parts.description);
		result.append("activationType");
		result.append(parts.meleeAbilityTiming);
		result.append("abilityClass");
		result.append(parts.abilityClass);
		result.append("visualCue");
		result.append(parts.visualCue);
		result.append("warmupCue");
		result.append(parts.warmupCue);
		result.append("icon");
		result.append(parts.icon);
		result.append("warmupTime");
		result.append(parts.warmup == "instant" || parts.warmup == "" || parts.warmup == "next" ? "0" : parts.warmup);
		result.append("cooldownTime");
		result.append(parts.cooldown == "instant" || parts.cooldown == "" ? "0" : parts.cooldown);
		result.append("cooldownCategory");
		result.append(parts.cooldownCategory);
		result.append("kind");
		result.append(parts.type);
		result.append("category");
		result.append(parts.category);
		result.append("tier");
		result.append(parts.tier);
		result.append("targetCriteria");
		result.append(this._dressFidelity(parts.targetFidelity, parts));

		if (::_Connection.getProtocolVersionId() >= 12)
		{
			result.append("powerLevel");

			if (parts.powerLevel == "")
			{
				parts.powerLevel = "1";
			}

			result.append(parts.powerLevel);
			result.append("buffCategory");
			result.append(parts.buffCategory);
		}

		if (parts.goldCost != "")
		{
			result.append("goldCost");
			result.append(parts.goldCost);
		}

		result.append("hostility");

		if (parts.targetFidelity == "Friend")
		{
			result.append("-1");
		}
		else if (parts.targetFidelity == "Enemy")
		{
			result.append("1");
		}
		else
		{
			result.append("0");
		}

		local activationCriteria = "";

		if (parts.allowedInCombat == "no")
		{
			activationCriteria += "NotStatus(IN_COMBAT),";
		}
		else if (parts.allowedInCombat == "ONLY")
		{
			activationCriteria += "HasStatus(IN_COMBAT),";
		}

		if ((parts.type == "magic" || parts.type == "Magic") && parts.category != "System")
		{
			activationCriteria += "NotSilenced(),";
		}

		if (parts.target == "ST" || parts.target.find("GTAE") != null || parts.target.find("STAE") != null)
		{
			if (parts.allowedInCombat == "yes")
			{
				activationCriteria += "Facing,";
			}
		}

		if (this.Util.trim(parts.range) != "")
		{
			activationCriteria += "InRange(" + this.Util.trim(parts.range) + "),";
		}

		if (::_Connection.getProtocolVersionId() >= 12)
		{
			if (parts.buffCategory != "")
			{
				activationCriteria += "CheckBuffLimits(" + parts.powerLevel + "," + parts.buffCategory + "),";
			}
		}

		if (parts.willCost != "")
		{
			activationCriteria += "Will(" + parts.willCost + "),";
		}

		if (parts.mightCost != "")
		{
			activationCriteria += "Might(" + parts.mightCost + "),";
		}

		if (parts.meleeExecuteCostMin != "")
		{
			if (parts.meleeExecuteCostMax != "")
			{
				activationCriteria += "MightCharge(" + parts.meleeExecuteCostMin + "," + parts.meleeExecuteCostMax + "),";
			}
			else
			{
				activationCriteria += "MightCharge(" + parts.meleeExecuteCostMin + "),";
			}
		}
		else if (parts.meleeExecuteCostMax != "")
		{
			activationCriteria += "MightCharge(" + parts.meleeExecuteCostMax + "," + parts.meleeExecuteCostMax + "),";
		}

		if (parts.magicExecuteCostMin != "")
		{
			if (parts.magicExecuteCostMax != "")
			{
				activationCriteria += "WillCharge(" + parts.magicExecuteCostMin + "," + parts.magicExecuteCostMax + "),";
			}
			else
			{
				activationCriteria += "WillCharge(" + parts.magicExecuteCostMin + "),";
			}
		}
		else if (parts.magicExecuteCostMax != "")
		{
			activationCriteria += "WillCharge(" + parts.magicExecuteCostMax + "," + parts.magicExecuteCostMax + "),";
		}

		activationCriteria += parts.specialRequirement;
		result.append("activationCriteria");

		if (activationCriteria != "")
		{
			result.append(activationCriteria[activationCriteria.len() - 1] == "," ? activationCriteria.slice(0, -1) : activationCriteria);
		}
		else
		{
			result.append("");
		}

		local skillsRequired = "(";
		skillsRequired += parts.previousSkillRequired;
		skillsRequired = this.Util.replace(skillsRequired, ",", "|");
		skillsRequired = this.Util.replace(skillsRequired, " ", "");
		skillsRequired += ")";
		result.append("prerequisites");
		result.append(parts.level + "," + parts.apCostCrossClass + "," + parts.apCostInClass + "," + skillsRequired + "," + parts.useableBy);
		result.append("usetype");
		result.append(parts.useType);
		result.append("addMeleeCharge");
		result.append(parts.addMeleeCharge == "" ? "0" : parts.addMeleeCharge);
		result.append("addMagicCharge");
		result.append(parts.addMagicCharge == "" ? "0" : parts.addMagicCharge);
		local actions = "";

		foreach( k, v in parts.events )
		{
			if (v.actions != "")
			{
				local name = "source" in v ? k + v.source : k;
				actions += name + ":" + v.target + ":" + v.actions + ";";
			}
		}

		result.append("actions");
		result.append(actions.len() > 0 ? actions.slice(0, -1) : "");
		return result;
	}

	function importAbilityLine( line )
	{
		if (line == "")
		{
			return;
		}

		local tokenCount = 54;
		local tokens = this.Util.split(line, "\t");

		if (tokens.len() != tokenCount)
		{
			this.mImportName = "Line " + (this.mImportIndex + 2);
			this.onQueryError(null, "Invalid line formatting (has " + tokens.len() + " fields, but expecting " + tokenCount + " ): " + line);
			return;
		}

		local x;

		for( x = 0; x < tokens.len(); x++ )
		{
			tokens[x] = this.Util.trim(tokens[x]);
		}

		if (tokens[0] == "" || tokens[2] == "")
		{
			this._importNext();
			return;
		}

		local parts = {};
		parts.abilityId <- tokens[0];
		parts.groupId <- tokens[1];
		parts.name <- tokens[2];
		parts.grade <- tokens[3];
		parts.meleeAbilityTiming <- tokens[4];
		parts.warmupCue <- tokens[5];
		parts.visualCue <- tokens[6];
		parts.slotCoordinate <- tokens[7];
		parts.icon <- tokens[8];
		parts.description <- tokens[9];
		parts.abilityClass <- tokens[10];
		parts.tier <- tokens[11];
		parts.type <- tokens[12];
		parts.useableBy <- tokens[13];
		parts.targetFidelity <- tokens[14];
		parts.powerLevel <- tokens[15];
		parts.buffCategory <- this.Util.replace(tokens[16], "\"", "");
		parts.category <- tokens[17];
		parts.level <- tokens[18];
		parts.apCostCrossClass <- tokens[19];
		parts.apCostInClass <- tokens[20];
		parts.previousSkillRequired <- tokens[21];
		parts.reagentType <- tokens[22];
		parts.specialRequirement <- tokens[23];
		parts.useType <- tokens[24];
		parts.allowedInCombat <- tokens[25];
		parts.range <- tokens[26];
		parts.warmup <- tokens[27];
		parts.cooldown <- tokens[28];
		parts.cooldownCategory <- tokens[29];
		parts.addMeleeCharge <- tokens[30];
		parts.meleeExecuteCostMin <- tokens[31];
		parts.meleeExecuteCostMax <- tokens[32];
		parts.addMagicCharge <- tokens[33];
		parts.magicExecuteCostMin <- tokens[34];
		parts.magicExecuteCostMax <- tokens[35];
		parts.mightCost <- tokens[36];
		parts.willCost <- tokens[37];
		parts.goldCost <- tokens[38];
		parts.target <- tokens[39];
		parts.duration <- tokens[41];
		parts.interval <- tokens[42];
		local events = {};
		events.onActivate <- {
			target = parts.target,
			actions = tokens[40]
		};
		events.onIterate <- {
			target = tokens[43],
			actions = tokens[44]
		};
		events.onParry <- {
			target = tokens[45],
			actions = tokens[46]
		};
		events.onDamage <- {
			source = tokens[47],
			target = tokens[48],
			actions = tokens[49]
		};
		events.onHit <- {
			source = tokens[50],
			target = tokens[51],
			actions = tokens[52]
		};
		parts.events <- events;
		this.mImportName = "Ability " + parts.abilityId + " (" + parts.name + ")";
		local query = this.compileAbilityQuery(parts);
		this._Connection.sendQuery("ab.edit", this, query);
	}

	function _importNextAbility()
	{
		if (this.mImportIndex < this.mImportDefLines.len())
		{
			this.importAbilityLine(this.mImportDefLines[this.mImportIndex++]);
			return;
		}

		if (this.mImportIndex >= this.mImportDefLines.len())
		{
			this.postOutput("Import Complete");
			local str = "Import complete. (";
			str += this.mImportChanges.len() + " modified";

			if (this.mImportErrors)
			{
				str += ", " + this.mImportErrors + " error(s)";
			}

			str += ".)";
			this.mStatusLabel.setText(str);
			this.mImportButton.setText("Import");
			this.mImportIndex = null;

			if (this.mImportChanges.len() > 0)
			{
				::_AbilityManager.refreshAbs(this.Util.join(this.mImportChanges, " "));
			}

			::_Connection.sendQuery("updateContent", this, [
				"abilities"
			]);
			::_AbilityHelper.updateClientAbilities();
			return;
		}
	}

	function _importNext()
	{
		if (this.mImportIndex == null)
		{
			return;
		}

		this.mStatusLabel.setText("Importing " + this.mImportIndex);

		switch(this.mEditingType)
		{
		case this.ABILITY:
			this._importNextAbility();
			break;

		case this.STAT_KIT:
			this._importNextStatKit();
			break;

		case this.AI_PACKAGE:
			this._importNextAIPackage();
			break;

		case this.SPAWN_PACKAGE:
			this._importNextSpawnPackage();
			break;

		case this.SPAWN_TABLE:
			this._importNextSpawnTable();
			break;

		case this.ITEM:
			this._importNextItem();
			break;

		case this.BODY_TYPE:
			this._importNextBodyType();
			break;

		case this.CREDIT_ITEM:
			this._importNextCreditItem();
			break;

		case this.SPECIAL_OFFER:
			this._importNextSpecialOffer();
			break;
		}
	}

	function onQueryComplete( qa, rows )
	{
		if (this.mImportIndex == null)
		{
			return;
		}

		this._importNext();
	}

	function onQueryError( qa, error )
	{
		if (this.mImportIndex == null)
		{
			return;
		}

		local errtext = "";

		foreach( err in this.Util.split(error, "\n") )
		{
			this.mImportErrors++;
			errtext += this.mImportName + ": " + err;
		}

		this.postError(errtext);
		this.mImportButton.setText("Cancel (" + this.mImportErrors + " Errors)");
		this._importNext();
	}

}

