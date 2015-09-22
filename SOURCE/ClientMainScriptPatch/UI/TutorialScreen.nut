this.MovementState <- {
	ON_SCREEN = 0,
	OFF_SCREEN = 1,
	REPOSITION = 2
};
this.TutorialType <- {
	WELCOME_MOVEMENT = 0,
	TALK_TO_SHROOMIE = 1,
	QUEST_INDICATOR = 2,
	INTERFACE = 3,
	AUTO_ATTACK = 4,
	LOOT = 5,
	EQUIPPING = 6,
	PLAYER_STATUS = 7,
	FINDING_CAMP = 8,
	FIXED_BOAT = 9,
	MINIMAP = 10,
	HEROISM = 11,
	DEATH = 12,
	BINDING = 13,
	ENTER_LIGHTHOUSE = 14,
	COPPER_SHOP = 15,
	HENGE = 16,
	HEROIC_MOB = 17,
	LOOTED_ESSENCE = 18,
	GAINED_BAG = 19,
	ACHIEVED_LVL_3 = 20,
	CRAFTING = 21,
	ARMOR_REFASHIONER = 22,
	CHARMS = 23,
	PARTY_INVITE = 24,
	FRIEND_INVITE = 25,
	CLAN_INVITE = 26,
	CREDIT_SHOP = 27,
	FIRST_QUEST = 28,
	HELP = 29
};
class this.TutorialManager 
{
	mNextXPos = 0;
	mNextYPos = 0;
	mTutorialsToGoOut = null;
	mTutorialsToGoIn = null;
	mTutorialsToGoDown = null;
	mMovementState = null;
	mAllDisplayedTutorials = null;
	mTutorialsActive = false;
	mTutorials = null;
	mWelcomeTutorial = null;
	mMovementTutorial = null;
	mPollEvent = null;
	mLastTime = 0.0;
	constructor()
	{
		this.mNextXPos = 5;
		this.mNextYPos = ::Screen.getHeight() / 2 - 24;
		this.mTutorialsToGoOut = [];
		this.mTutorialsToGoIn = [];
		this.mTutorialsToGoDown = [];
		this.mAllDisplayedTutorials = [];
		this.mTutorials = {};
		this.setTutorialsActive(::Pref.get("tutorial.active"));
		local displayTuturial = ::Pref.get("tutorial.diaplayTutorial");
		this.updateDisplayedTutorials(displayTuturial);
		::_root.addListener(this);
	}

	function createTutorials()
	{
		if (this.mTutorials.len() > 0)
		{
			return;
		}

		local callback = {
			function shouldDisplayTutorial()
			{
				return true;
			}

		};
		local tutorialImage = this.GUI.Image("WASDTutorial");
		local newTutorial = this.Tutorial("Movement", tutorialImage, this.TutorialType.WELCOME_MOVEMENT, callback, 476, 365);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.WELCOME_MOVEMENT] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Welcome", "<font size=\"20\">Walk over to Redcap the Shroomie and <font color=\"FFFF99\"><b>Right-Click</b></font> to start the tutorial!</font>", this.TutorialType.TALK_TO_SHROOMIE, null, 200, 120);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.TALK_TO_SHROOMIE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("FirstQuestTutorial");
		newTutorial = this.Tutorial("Welcome", tutorialImage, this.TutorialType.FIRST_QUEST, null, 476, 365);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.FIRST_QUEST] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("QuestIndicatorTutorial");
		newTutorial = this.Tutorial("Quests", tutorialImage, this.TutorialType.QUEST_INDICATOR, callback, 476, 365);
		this.mTutorials[this.TutorialType.QUEST_INDICATOR] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("InterfaceTutorial");
		newTutorial = this.Tutorial("Interface", tutorialImage, this.TutorialType.INTERFACE, null, 620, 505);
		this.mTutorials[this.TutorialType.INTERFACE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("AutoAttackTutorial");
		newTutorial = this.Tutorial("Combat", tutorialImage, this.TutorialType.AUTO_ATTACK, null, 476, 365);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.AUTO_ATTACK] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("LootingTutorial");
		newTutorial = this.Tutorial("Loot", tutorialImage, this.TutorialType.LOOT, null, 476, 375);
		this.mTutorials[this.TutorialType.LOOT] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("EquipmentTutorial");
		newTutorial = this.Tutorial("Equipment", tutorialImage, this.TutorialType.EQUIPPING, null, 476, 375);
		this.mTutorials[this.TutorialType.EQUIPPING] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("PlayerStatusTutorial");
		newTutorial = this.Tutorial("Combat", tutorialImage, this.TutorialType.PLAYER_STATUS, null, 476, 375);
		this.mTutorials[this.TutorialType.PLAYER_STATUS] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Interacting", "<font size=\"20\">You have found the camp. To take the hammer you must <font color=\"FFFF99\"><b>Right-Click</b></font> on it and wait until the interaction bar has emptied before moving or doing any other action.</font>", this.TutorialType.FINDING_CAMP, null, 300, 145);
		this.mTutorials[this.TutorialType.FINDING_CAMP] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Good Luck", "<font size=\"20\">You have completed the basic tutorial. As you play through the world of Earth Eternal and experience new things additional helpful tips will popup to help you!<br>" + "You can turn off these Tutorial Tips in the Options Menu.<br>" + "Good luck Adventurer!<br></font>", this.TutorialType.FIXED_BOAT, null, 300, 210);
		this.mTutorials[this.TutorialType.FIXED_BOAT] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial.setAutoPopup(true);
		tutorialImage = this.GUI.Image("MiniMapTutorial");
		newTutorial = this.Tutorial("Quests", tutorialImage, this.TutorialType.MINIMAP, null, 476, 375);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.MINIMAP] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("TutorialHeroism");
		newTutorial = this.Tutorial("Heroism", tutorialImage, this.TutorialType.HEROISM, null, 475, 385);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.HEROISM] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Death", "<font size=\"20\">You have died. There are three options for you to return to life.<br><br>" + "<font color=\"FFFF99\"><b>Revive</b></font> - when you come back to life you will have all of your main statistics reduced by 50%, you lose all of your Heroism, but there is no fee.<br><br>" + "<font color=\"FFFF99\"><b>Resurrect</b></font> - when you come back to life you will lose 50% of your Heroism but there is small fee.<br><br>" + "<font color=\"FFFF99\"><b>Rebirth</b></font> - when you come back to life you lose 10% of your Heroism but there is a large fee.<br><br>" + "You will come back to life at the nearest Sanctuary location (the stone platform with the golden Ankh).</font>", this.TutorialType.DEATH, null, 300, 390);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.DEATH] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Binding", "<font size=\"20\">You have just used the Bind ability.<br>" + "You may only use bind within 250 meters of a Sanctuary location (the stone platform with the golden Ankh).<br>" + "Once you successfully Bind you will be able to use <font color=\"FFFF99\"><b>Portal: Bind</b></font> to return to that location.</font>", this.TutorialType.BINDING, null, 400, 165);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.BINDING] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Spawners", "<font size=\"20\">In order to make <font color=\"FFFF99\"><b>Anglor Dren</b></font> spawn you will first have to kill all of the <font color=\"FFFF99\"><b>Madvines</b></font>. " + "This is a step-up spawner - a series of spawners that change as you kill creatures. <br>Each kill in a step-up spawner will add points to the overall spawner. " + "Once the points add up to a specific total the spawner will begin spawning the next step in the series. You will experience more of these as you play the game.</font>", this.TutorialType.ENTER_LIGHTHOUSE, null, 400, 200);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.ENTER_LIGHTHOUSE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Shops", "<font size=\"20\">This is an NPC Shop. " + "You can purchase items by \"grabbing\" the item with you <font color=\"FFFF99\"><b>Left Mouse Button</b></font> and dragging it to your Inventory.<br><br>" + "You can also purchase by holding the <font color=\"FFFF99\"><b>Control Key</b></font> and then <font color=\"FFFF99\"><b>Right-Clicking</b></font> on the item you wish to buy.<br><br>" + "You may sell items by \"grabbing\" the item with you <font color=\"FFFF99\"><b>Left Mouse Button</b></font> and dragging it to the Shop window.</font>", this.TutorialType.COPPER_SHOP, null, 400, 200);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.COPPER_SHOP] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Henge", "<font size=\"20\"><font color=\"FFFF99\"><b>Right-Click</b></font> the Henge to add it to your known list of Henges!<br>" + "You can travel between Henges that you have discovered for a small fee.<br>" + "If you have purchased a trip to another Henge and you are inside the radius of the Henge you will be transported to the Henge location you chose once it triggers.<br>" + "The Henge triggers every 3 Minutes.</font>", this.TutorialType.HENGE, null, 400, 210);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.HENGE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Heroic Mob", "<font size=\"20\">The creature you just targeted is <font color=\"FFFF99\"><b>Heroic</b></font>. Heroic creatures take damage and deal damage just like a normal creature but they have increased Health making them harder to kill.</font>", this.TutorialType.HEROIC_MOB, null, 300, 150);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.HEROIC_MOB] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Boss Loot", "<font size=\"20\">You have received a <font color=\"FFFF99\"><b>Treasure Token</b></font>. " + "The Token is a type of currency than be spent in <font color=\"FFFF99\"><b>Quest Reward Chests</b></font> and some <font color=\"FFFF99\"><b>Boss Creature Chests</b></font>. " + "Unless otherwise stated each Quest / Boss Token can only be used at the specific Chest for that Quest or Boss. " + "<font color=\"FFFF99\"><b>Right-Click</b></font> on the chest and make your choice!</font>", this.TutorialType.LOOTED_ESSENCE, null, 400, 160);
		newTutorial.setAutoPopup(true);
		this.mTutorials[this.TutorialType.LOOTED_ESSENCE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Extra Bags", "<font size=\"20\">You just received an <font color=\"FFFF99\"><b>Extra Inventory Bag</b></font>. " + "This will allow you to increase the total space in your Inventory. " + "Drag the bag from your Inventory to one of the four small bag locations located at the bottom of you Inventory. " + "This is not permanent - you can find/purchase bigger bags and replace any bag you are currently using.</font>", this.TutorialType.GAINED_BAG, null, 400, 180);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.GAINED_BAG] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Crafting NPC", "<font size=\"20\">Welcome to the Crafting NPC.<br>" + "Crafting requires several things.<br>" + "A <font color=\"FFFF99\"><b>Recipe</b></font> - is a list of items needed to create the final Recipe Item. It should be placed in the top left slot.<br><br>" + "<font color=\"FFFF99\"><b>Key Component</b></font> - this is a part of the Recipe you can purchase from the <font color=\"FFFF99\"><b>Crafting Supply Vendor</b></font>.<br><br>" + "<font color=\"FFFF99\"><b>Components</b></font> - these are various items that are used for making a specific item.<br><br>" + "Once you place the Recipe in the correct slot it will list all the items you need and show you which of those items you currently have in your Inventory.<br>" + "If you have all the items you can complete the item and it will be placed in your Inventory.</font>", this.TutorialType.CRAFTING, null, 400, 385);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.CRAFTING] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Armor Refashioner", "<font size=\"20\">Welcome to the Armor Refashioner.<br><br>" + "You can take any two same Armor Items, of the same type and move the look of one to the other while keeping the statistics of the second item.<br><br>" + "This means you have a cool looking pair of pants but another pair that is not as cool looking but has great stats you can swap the look and now have the greats stats AND the cool look!</font>", this.TutorialType.ARMOR_REFASHIONER, null, 300, 290);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.ARMOR_REFASHIONER] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Charms", "<font size=\"20\">Charms are items that add large bonuses to your stats.<br>" + "There are six colors of Charms.<br>" + "Basic Charms are <font color=\"FF0000\">Red</font>, <font color=\"0000FF\">Blue</font> and <font color=\"FFFF00\">Yellow</font>.<br>" + "Advanced Charms are <font color=\"800080\">Purple</font>, <font color=\"FFA500\">Orange</font> and <font color=\"008000\">Green</font>.<br>" + "<font color=\"FF0000\">Red Charms</font> can be put in <font color=\"FF0000\">Red</font>, <font color=\"800080\">Purple</font>, or <font color=\"FFA500\">Orange</font> slots.<br>" + "<font color=\"0000FF\">Blue Charms</font> can be put in <font color=\"0000FF\">Blue</font>, <font color=\"800080\">Purple</font>, or <font color=\"008000\">Green</font> slots.<br>" + "<font color=\"FFFF00\">Yellow Charms</font> can be put in <font color=\"FFFF00\">Yellow</font>, <font color=\"008000\">Green</font>, or <font color=\"FFA500\">Orange</font> slots." + "</font>", this.TutorialType.CHARMS, null, 365, 210);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.CHARMS] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Party Invite", "<font size=\"20\">You have been invited to join a Party.<br>" + "If you accept you will be able to adventure with the other people in the Party.<br>" + "You will share Experience and Completing Quest Objectives.<br>" + "You can also enter Instanced Spaces (dungeons) with those in your group." + "</font>", this.TutorialType.PARTY_INVITE, null, 300, 200);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.PARTY_INVITE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Social", "<font size=\"20\">You have just received a Friend Invite. Press the <font color=\"FFFF99\"><b>Social Options</b></font> on the <font color=\"FFFF99\"><b>Main Function Bar</b></font> to see a list of your friends.<br>" + "Once you are a friend of another player both you and your new friend will be able to see if each other are online in the friends list.  From there, you can easily send messages to each other.<br>" + "</font>", this.TutorialType.FRIEND_INVITE, null, 300, 200);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.FRIEND_INVITE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Social", "<font size=\"20\">You have just received a Clan Invite.<br>" + "Clans are social groups of likeminded people who wish to associate together.<br>" + "It is best to get to know people before you join their Clan.<br>" + "You can see other members of your Clan by opening the <font color=\"FFFF99\"><b>Social Options</b></font> on the <font color=\"FFFF99\"><b>Main Function Bar</b></font>.<br>" + "</font>", this.TutorialType.CLAN_INVITE, null, 330, 200);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.CLAN_INVITE] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		newTutorial = this.Tutorial("Credit Shop", "<font size=\"20\">You have opened the <font color=\"FFFF99\"><b>Credit Shop</b></font>.<br>" + "The <font color=\"FFFF99\"><b>Credit Shop</b></font> is where you can spend Earth Eternal Credits to purchase items such as powerful potions, amazing Charms to boost your stats, new awesome looks for you armor and much more.<br>" + 
			"In order to purchase items from the Shop you must first earn Credits.<br>" +
			"Credits may be earned in a number of ways, including logging in every day,<br>" + 
			"killing certain creatures and others. Get credit bonuses by partying with other<br>" +  
			"players</font>", this.TutorialType.CREDIT_SHOP, null, 450, 225);
		newTutorial.setAutoPopup(false);
		this.mTutorials[this.TutorialType.CREDIT_SHOP] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
		tutorialImage = this.GUI.Image("HelpTutorial");
		newTutorial = this.Tutorial("Help", tutorialImage, this.TutorialType.HELP, null, 470, 365);
		this.mTutorials[this.TutorialType.HELP] <- {
			seen = false,
			component = newTutorial,
			out = false
		};
	}

	function displayTutorial( tutorial )
	{
		local currentTutorialType = tutorial.component.getType();

		foreach( oldTutorial in this.mAllDisplayedTutorials )
		{
			if (oldTutorial.component.getType() == currentTutorialType)
			{
				return;
			}
		}

		this._enterFrameRelay.addListener(this);
		tutorial.out = true;
		this.mTutorialsToGoOut.push(tutorial);
		this.mAllDisplayedTutorials.push(tutorial);
		tutorial.component.setVisible(false);
		tutorial.component.setButtonPosition(-50, this.mNextYPos);
		tutorial.component.setDestinationPos(this.mNextXPos, this.mNextYPos);
		tutorial.component.setOverlay("GUI/TutorialOverlay");
		this.mNextYPos -= 50;
	}

	function onAbilityActivated( abilityID )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (abilityID == 188)
		{
			if (!this.mTutorials[this.TutorialType.BINDING].seen && !this.mTutorials[this.TutorialType.BINDING].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.BINDING]);
			}
		}
	}

	function onCreatureUsed( creatureID, typeOfUse )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		switch(typeOfUse)
		{
		case "CopperShop":
			if (!this.mTutorials[this.TutorialType.COPPER_SHOP].seen && !this.mTutorials[this.TutorialType.COPPER_SHOP].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.COPPER_SHOP]);
			}

			break;

		case "Crafter":
			if (!this.mTutorials[this.TutorialType.CRAFTING].seen && !this.mTutorials[this.TutorialType.CRAFTING].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.CRAFTING]);
			}

			break;

		case "Transformer":
			if (!this.mTutorials[this.TutorialType.ARMOR_REFASHIONER].seen && !this.mTutorials[this.TutorialType.ARMOR_REFASHIONER].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.ARMOR_REFASHIONER]);
			}

			break;
		}
	}

	function onItemGained( itemDef, id )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (id == 21275)
		{
			if (!this.mTutorials[this.TutorialType.LOOTED_ESSENCE].seen && !this.mTutorials[this.TutorialType.LOOTED_ESSENCE].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.LOOTED_ESSENCE]);
			}
		}
		else
		{
		}
	}

	function onEnterFrame()
	{
		if (this.LoadScreen.isVisible())
		{
			return;
		}

		local timeDifference = this.System.currentTimeMillis() - this.mLastTime;

		if (timeDifference > 4000)
		{
			timeDifference = 4000;
		}

		this.mLastTime = this.System.currentTimeMillis();

		if (this.mMovementState == this.MovementState.ON_SCREEN)
		{
			local tutorialsToRemove = [];

			if (this.mTutorialsToGoOut.len() > 0)
			{
				for( local i = 0; i < this.mTutorialsToGoOut.len(); i++ )
				{
					if (!("component" in this.mTutorialsToGoOut[i]))
					{
					}
					else
					{
						local component = this.mTutorialsToGoOut[i].component;
						component.setVisible(true);
						local currentPosition = component.getButtonPosition();
						local destPos = component.getDestinationPos();

						if (currentPosition.x >= destPos.x)
						{
							this.mTutorialsToGoOut[i].component.setButtonPosition(destPos.x, destPos.y);
							tutorialsToRemove.push(this.mTutorialsToGoOut[i]);
							component.onTutorialReachedDestination();
						}
						else
						{
							currentPosition.x += timeDifference * 0.25;
							this.mTutorialsToGoOut[i].component.setButtonPosition(currentPosition.x, currentPosition.y);
						}
					}
				}

				foreach( tutorial in tutorialsToRemove )
				{
					for( local i = 0; i < this.mTutorialsToGoOut.len(); i++ )
					{
						if (tutorial == this.mTutorialsToGoOut[i])
						{
							this.mTutorialsToGoOut.remove(i);
						}
					}
				}
			}
			else
			{
				this.mMovementState = null;
			}
		}
		else if (this.mMovementState == this.MovementState.OFF_SCREEN)
		{
			local tutorialsToRemove = [];

			if (this.mTutorialsToGoIn.len() > 0)
			{
				for( local i = 0; i < this.mTutorialsToGoIn.len(); i++ )
				{
					local component = this.mTutorialsToGoIn[i].component;
					local currentPosition = component.getButtonPosition();

					if (currentPosition.x <= component.getDestinationPos().x)
					{
						tutorialsToRemove.push(this.mTutorialsToGoIn[i]);
					}
					else
					{
						currentPosition.x -= 1 * (timeDifference * 0.25);
						component.setButtonPosition(currentPosition.x, currentPosition.y);
					}
				}

				foreach( tutorial in tutorialsToRemove )
				{
					for( local i = 0; i < this.mTutorialsToGoIn.len(); i++ )
					{
						if (tutorial == this.mTutorialsToGoIn[i])
						{
							this.mTutorialsToGoIn.remove(i);
							local foundTutorial = false;

							for( local j = 0; j < this.mAllDisplayedTutorials.len(); j++ )
							{
								if (!foundTutorial && this.mAllDisplayedTutorials[j] == tutorial)
								{
									tutorial.component.setVisible(false);
									this.mAllDisplayedTutorials.remove(j);
									foundTutorial = true;
								}
							}
						}
					}
				}
			}
			else
			{
				this.mNextXPos = 5;
				this.mNextYPos = ::Screen.getHeight() / 2 - 24;

				for( local j = 0; j < this.mAllDisplayedTutorials.len(); j++ )
				{
					this.mAllDisplayedTutorials[j].component.setDestinationPos(this.mNextXPos, this.mNextYPos);
					this.mTutorialsToGoDown.push(this.mAllDisplayedTutorials[j]);
					this.mNextYPos -= 50;
				}

				this.mMovementState = this.MovementState.REPOSITION;
			}
		}
		else if (this.mMovementState == this.MovementState.REPOSITION)
		{
			local tutorialsToRemove = [];

			if (this.mTutorialsToGoDown.len() > 0)
			{
				for( local i = 0; i < this.mTutorialsToGoDown.len(); i++ )
				{
					local component = this.mTutorialsToGoDown[i].component;
					local currentPosition = component.getButtonPosition();
					local destPos = component.getDestinationPos();

					if (currentPosition.y >= destPos.y)
					{
						component.setButtonPosition(destPos.x, destPos.y);
						tutorialsToRemove.push(this.mTutorialsToGoDown[i]);
					}
					else
					{
						currentPosition.y += 1 * (timeDifference * 0.25);
						component.setButtonPosition(currentPosition.x, currentPosition.y);
					}
				}

				foreach( tutorial in tutorialsToRemove )
				{
					for( local i = 0; i < this.mTutorialsToGoDown.len(); i++ )
					{
						if (tutorial == this.mTutorialsToGoDown[i])
						{
							this.mTutorialsToGoDown.remove(i);
						}
					}
				}
			}
			else
			{
				this.mMovementState = null;
			}
		}

		if (this.mMovementState == null)
		{
			if (this.mTutorialsToGoIn.len() > 0)
			{
				this.mMovementState = this.MovementState.OFF_SCREEN;
			}
			else if (this.mTutorialsToGoOut.len() > 0)
			{
				this.mMovementState = this.MovementState.ON_SCREEN;
			}
			else
			{
				this._enterFrameRelay.removeListener(this);
			}
		}

		for( local j = 0; j < this.mAllDisplayedTutorials.len(); j++ )
		{
			this.mAllDisplayedTutorials[j].component.pulsate();
		}
	}

	function onRemoveAndSaveTutorial()
	{
		local tutorialType = [];

		foreach( tutorial in this.mAllDisplayedTutorials )
		{
			tutorialType.append(tutorial.component.getType());
		}

		::Pref.set("tutorial.diaplayTutorial", ::serialize(tutorialType), true, false);

		foreach( tutorial in this.mTutorials )
		{
			tutorial.component.setVisible(false);
			delete tutorial.component;
		}

		this.mTutorials = {};
		this.mAllDisplayedTutorials.clear();
		this.mAllDisplayedTutorials = [];
	}

	function onScreenResize()
	{
		this.mNextXPos = 5;
		this.mNextYPos = ::Screen.getHeight() / 2 - 24;

		for( local j = 0; j < this.mAllDisplayedTutorials.len(); j++ )
		{
			this.mAllDisplayedTutorials[j].component.setButtonPosition(this.mNextXPos, this.mNextYPos);
			this.mNextYPos -= 50;
		}
	}

	function onSocialEvent( event )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		switch(event)
		{
		case "PartyInvite":
			if (!this.mTutorials[this.TutorialType.PARTY_INVITE].seen && !this.mTutorials[this.TutorialType.PARTY_INVITE].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.PARTY_INVITE]);
			}

			break;

		case "FriendInvite":
			if (!this.mTutorials[this.TutorialType.FRIEND_INVITE].seen && !this.mTutorials[this.TutorialType.FRIEND_INVITE].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.FRIEND_INVITE]);
			}

			break;

		case "ClanInvite":
			if (!this.mTutorials[this.TutorialType.CLAN_INVITE].seen && !this.mTutorials[this.TutorialType.CLAN_INVITE].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.CLAN_INVITE]);
			}

			break;
		}
	}

	function onScreenOpened( screenName )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		switch(screenName)
		{
		case "Charms":
			if (!this.mTutorials[this.TutorialType.CHARMS].seen && !this.mTutorials[this.TutorialType.CHARMS].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.CHARMS]);
			}

			break;

		case "CreditShop":
			if (!this.mTutorials[this.TutorialType.CREDIT_SHOP].seen && !this.mTutorials[this.TutorialType.CREDIT_SHOP].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.CREDIT_SHOP]);
			}

			break;
		}
	}

	function onStatUpdated( statDef, newValue, oldValue )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (statDef == this.Stat.HEROISM)
		{
			if (newValue && newValue > 250)
			{
				if (!this.mTutorials[this.TutorialType.HEROISM].seen && !this.mTutorials[this.TutorialType.HEROISM].out)
				{
					this.displayTutorial(this.mTutorials[this.TutorialType.HEROISM]);
				}
			}
		}
	}

	function onStatusEffectSet( statusEffect )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (statusEffect == this.StatusEffects.DEAD)
		{
			if (!this.mTutorials[this.TutorialType.DEATH].seen && !this.mTutorials[this.TutorialType.DEATH].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.DEATH]);
			}
		}
	}

	function onTargetSelected( creature )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		local rarity = creature.getStat(this.Stat.RARITY);

		if (rarity && rarity == this.CreatureRarityType.HEROIC)
		{
			if (!this.mTutorials[this.TutorialType.HEROIC_MOB].seen && !this.mTutorials[this.TutorialType.HEROIC_MOB].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.HEROIC_MOB]);
			}
		}
	}

	function onZoneUpdate( zoneDefId )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (zoneDefId == 77)
		{
			if (!this.mTutorials[this.TutorialType.ENTER_LIGHTHOUSE].seen && !this.mTutorials[this.TutorialType.ENTER_LIGHTHOUSE].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.ENTER_LIGHTHOUSE]);
			}
		}
	}

	function onTutorialPoll()
	{
		if (this.LoadScreen.isVisible())
		{
			return;
		}

		if (::_avatar == null)
		{
			return;
		}

		local nearbyCreatures = ::_sceneObjectManager.getCreatures();
		local avatarPos = ::_avatar.getPosition();

		foreach( creature in nearbyCreatures )
		{
			if (this.Math.manhattanDistanceXZ(creature.getPosition(), avatarPos) < 100)
			{
				if (creature.hasStatusEffect(this.StatusEffects.HENGE))
				{
					if (!this.mTutorials[this.TutorialType.HENGE].seen && !this.mTutorials[this.TutorialType.HENGE].out)
					{
						this.displayTutorial(this.mTutorials[this.TutorialType.HENGE]);
					}
				}
			}
		}

		foreach( tutorial in this.mTutorials )
		{
			if (tutorial.out == false && tutorial.seen == false && tutorial.component.shouldDisplayTutorial())
			{
				this.displayTutorial(tutorial);
			}
		}
	}

	function resetTutorials()
	{
		::Pref.set("tutorial.seen", "", true, false);

		foreach( tutorial in this.mTutorials )
		{
			tutorial.seen = false;
		}
	}

	function removeTutorial( tutorial )
	{
		this._enterFrameRelay.addListener(this);
		local tutorialType = tutorial.getType();
		this.mTutorials[tutorialType].seen = true;
		this.mTutorials[tutorialType].out = false;
		this.mTutorialsToGoIn.push(this.mTutorials[tutorialType]);
		local currentPosition = tutorial.getPosition();
		tutorial.setDestinationPos(currentPosition.x - 40, ::Screen.getHeight());
		this.serialize();

		if (tutorialType == this.TutorialType.WELCOME_MOVEMENT)
		{
			local currentZone = ::_avatar.getZoneDefId();

			if (currentZone == 59)
			{
				this.mTutorials[this.TutorialType.TALK_TO_SHROOMIE].component.setForceShowTutorial(true);
			}
			else if (currentZone == 92)
			{
				this.mTutorials[this.TutorialType.FIRST_QUEST].component.setForceShowTutorial(true);
			}
		}
		else
		{
		}
	}

	function setTutorialsActive( value )
	{
		if (this.mTutorialsActive != value && value)
		{
			this.mTutorialsActive = value;
			this.createTutorials();
			this.unserialize(::Pref.get("tutorial.seen"));

			if (this.mPollEvent == null)
			{
				this.mPollEvent = ::_eventScheduler.repeatIn(1.0, 1.0, this, "onTutorialPoll");
			}
		}
		else
		{
			::_eventScheduler.cancel(this.mPollEvent);
			this.mPollEvent = null;
		}
	}

	function serialize()
	{
		local tutorialsSeen = [];

		foreach( tutorial in this.mTutorials )
		{
			if (tutorial.seen == true)
			{
				tutorialsSeen.append(tutorial.component.getType());
			}
		}

		::Pref.set("tutorial.seen", ::serialize(tutorialsSeen), true, false);
	}

	function questAccepted( questId )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (questId == 379 || questId == 381 || questId == 385 || questId == 383)
		{
			if (!this.mTutorials[this.TutorialType.AUTO_ATTACK].seen && !this.mTutorials[this.TutorialType.AUTO_ATTACK].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.AUTO_ATTACK]);
			}
		}
		else if (questId == 382 || questId == 380 || questId == 386 || questId == 384)
		{
			if (!this.mTutorials[this.TutorialType.PLAYER_STATUS].seen && !this.mTutorials[this.TutorialType.PLAYER_STATUS].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.PLAYER_STATUS]);
			}
		}
		else if (questId == 637)
		{
			if (!this.mTutorials[this.TutorialType.MINIMAP].seen && !this.mTutorials[this.TutorialType.MINIMAP].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.MINIMAP]);
			}
		}
		else if (questId == 638)
		{
			if (!this.mTutorials[this.TutorialType.AUTO_ATTACK].seen && !this.mTutorials[this.TutorialType.AUTO_ATTACK].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.AUTO_ATTACK]);
			}
		}
		else if (questId == 639)
		{
			if (!this.mTutorials[this.TutorialType.PLAYER_STATUS].seen && !this.mTutorials[this.TutorialType.PLAYER_STATUS].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.PLAYER_STATUS]);
			}
		}
		else if (questId == 641)
		{
			if (!this.mTutorials[this.TutorialType.HELP].seen && !this.mTutorials[this.TutorialType.HELP].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.HELP]);
			}
		}
		else if (questId == 649)
		{
			if (!this.mTutorials[this.TutorialType.EQUIPPING].seen && !this.mTutorials[this.TutorialType.EQUIPPING].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.EQUIPPING]);
			}
		}
	}

	function questCompleted( questId )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (questId == 378)
		{
			if (!this.mTutorials[this.TutorialType.INTERFACE].seen && !this.mTutorials[this.TutorialType.INTERFACE].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.INTERFACE]);
			}
		}
		else if (questId == 379 || questId == 381 || questId == 385 || questId == 383)
		{
			if (!this.mTutorials[this.TutorialType.EQUIPPING].seen && !this.mTutorials[this.TutorialType.EQUIPPING].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.EQUIPPING]);
			}
		}
		else if (questId == 353)
		{
			if (!this.mTutorials[this.TutorialType.GAINED_BAG].seen && !this.mTutorials[this.TutorialType.GAINED_BAG].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.GAINED_BAG]);
			}
		}
	}

	function lootDropped()
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (!this.mTutorials[this.TutorialType.LOOT].seen && !this.mTutorials[this.TutorialType.LOOT].out)
		{
			this.displayTutorial(this.mTutorials[this.TutorialType.LOOT]);
		}
	}

	function questObjectiveUpdated( questId, objective )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (objective == 0 && (questId == 382 || questId == 380 || questId == 386 || questId == 384))
		{
			if (!this.mTutorials[this.TutorialType.FINDING_CAMP].seen && !this.mTutorials[this.TutorialType.FINDING_CAMP].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.FINDING_CAMP]);
			}
		}
	}

	function questActCompleted( questId, act )
	{
		if (!this.mTutorialsActive)
		{
			return;
		}

		if (act == 2 && (questId == 382 || questId == 380 || questId == 386 || questId == 384))
		{
			if (!this.mTutorials[this.TutorialType.FIXED_BOAT].seen && !this.mTutorials[this.TutorialType.FIXED_BOAT].out)
			{
				this.displayTutorial(this.mTutorials[this.TutorialType.FIXED_BOAT]);
			}
		}
	}

	function unserialize( value )
	{
		if (value == "")
		{
			return;
		}

		local array = ::unserialize(value);

		foreach( seen in array )
		{
			if (seen in this.mTutorials)
			{
				this.mTutorials[seen].seen = true;
			}
		}
	}

	function updateDisplayedTutorials( value )
	{
		if (value == "")
		{
			return;
		}

		local tutorialTypeArray = ::unserialize(value);

		if (!this.mTutorialsActive)
		{
			return;
		}

		foreach( type in tutorialTypeArray )
		{
			if (this.mTutorials && type in this.mTutorials)
			{
				this.displayTutorial(this.mTutorials[type]);
			}
		}
	}

}

class this.Tutorial extends this.GUI.Container
{
	mName = "";
	mTutorialComponent = null;
	mButton = null;
	mGlowButton = null;
	mComponent = null;
	mDestinationXPos = 0;
	mDestinationYPos = 0;
	mCurrentButtonPositionX = 0;
	mCurrentButtonPositionY = 0;
	mType = -1;
	mForceShowTutorial = false;
	mDisplayCritera = "";
	mPopupMessageOpen = false;
	mTutorialPopup = null;
	mAcceptButton = null;
	mPopupWidth = 0;
	mPopupHeight = 0;
	mPulsate = 0.0;
	pulsateDirection = 0.1;
	mAutoPopup = false;
	constructor( name, tutorialComponent, type, displayCritera, width, height )
	{
		this.GUI.Container.constructor(this.GUI.BoxLayoutV());
		this.mName = this.GUI.Label(name);
		this.mButton = this.GUI.Button("");
		this.mButton.setAppearance("Tutorial/Notify");
		this.mButton.setFixedSize(32, 32);
		this.mGlowButton = this.GUI.Button("");
		this.mGlowButton.setAppearance("Tutorial/Highlight");
		this.mGlowButton.setFixedSize(32, 32);
		this.mGlowButton.setBlendColor(this.Color(0.89999998, 0.89999998, 0.89999998, 0.0));
		local buttonContainer = this.GUI.Container(null);
		buttonContainer.add(this.mButton);
		buttonContainer.add(this.mGlowButton);
		buttonContainer.setSize(32, 32);
		buttonContainer.setPreferredSize(32, 32);
		this.setSize(50, 50);
		this.mType = type;
		this.mDisplayCritera = displayCritera;
		this.add(this.mName);
		this.add(buttonContainer);
		this.mButton.setReleaseMessage("onClicked");
		this.mButton.addActionListener(this);
		this.mGlowButton.setReleaseMessage("onClicked");
		this.mGlowButton.addActionListener(this);
		this.mPopupWidth = width;
		this.mPopupHeight = height;

		if (typeof tutorialComponent == "string")
		{
			this.mTutorialComponent = this.GUI.HTML();
			this.mTutorialComponent.setText(tutorialComponent);
			this.mTutorialComponent.setMaximumSize(this.mPopupWidth, this.mPopupHeight);
		}
		else
		{
			this.mTutorialComponent = tutorialComponent;
		}

		this._exitGameStateRelay.addListener(this);
	}

	function onExitGame()
	{
		if (this.mTutorialPopup)
		{
			this.mTutorialPopup.destroy();
			this.mTutorialPopup = null;
		}

		this.destroy();
	}

	function getType()
	{
		return this.mType;
	}

	function setButtonPosition( xPos, yPos )
	{
		this.mCurrentButtonPositionX = xPos;
		this.mCurrentButtonPositionY = yPos;
		this.setPosition(xPos, yPos);
	}

	function getButtonPosition()
	{
		return {
			x = this.mCurrentButtonPositionX,
			y = this.mCurrentButtonPositionY
		};
	}

	function getName()
	{
		return this.mName;
	}

	function getTutorialComponent()
	{
		return this.mTutorialComponent;
	}

	function setDestinationPos( xPos, yPos )
	{
		this.mDestinationXPos = xPos;
		this.mDestinationYPos = yPos;
	}

	function setAutoPopup( value )
	{
		this.mAutoPopup = value;
	}

	function setForceShowTutorial( value )
	{
		this.mForceShowTutorial = value;
	}

	function getDestinationPos()
	{
		return {
			x = this.mDestinationXPos,
			y = this.mDestinationYPos
		};
	}

	function onAccept( button )
	{
		this.mPopupMessageOpen = false;
		this.mTutorialPopup.setOverlay(null);
		::_tutorialManager.removeTutorial(this);
	}

	function pulsate()
	{
		if (!this.mPopupMessageOpen)
		{
			this.mPulsate += this.pulsateDirection;

			if (this.pulsateDirection > 0.0)
			{
				if (this.mPulsate > 1.0)
				{
					this.mPulsate = 1.0;
					this.pulsateDirection = -0.025;
				}
			}
			else if (this.mPulsate <= 0.0)
			{
				this.mPulsate = 0.0;
				this.pulsateDirection = 0.025;
			}

			this.mGlowButton.setBlendColor(this.Color(0.89999998, 0.89999998, 0.89999998, this.mPulsate));
		}
	}

	function onClicked( button )
	{
		if (!this.mPopupMessageOpen)
		{
			local screenWidth = this.Screen.getWidth();
			local screenHeight = this.Screen.getHeight();
			this.mTutorialPopup = this.GUI.Container(this.GUI.GridLayout(2, 1));
			this.mTutorialPopup.getLayoutManager().setRows("*", 35);
			this.mTutorialPopup.setInsets(10, 10, 10, 10);
			this.mTutorialPopup.setAppearance("Panel");
			this.mAcceptButton = this.GUI.NarrowButton("OK!");
			this.mAcceptButton.setReleaseMessage("onAccept");
			this.mAcceptButton.addActionListener(this);
			this.mTutorialPopup.setOverlay("GUI/TutorialOverlay");
			this.mTutorialPopup.setPosition(screenWidth / 2 - this.mPopupWidth / 2, screenHeight / 2 - this.mPopupHeight / 2);
			this.mTutorialPopup.setSize(this.mPopupWidth, this.mPopupHeight);
			this.mTutorialPopup.setPreferredSize(this.mPopupWidth, this.mPopupHeight);
			this.mTutorialPopup.add(this.mTutorialComponent);
			this.mTutorialPopup.add(this.mAcceptButton);
			this.mPopupMessageOpen = true;
		}
	}

	function onTutorialReachedDestination()
	{
		if (this.mAutoPopup)
		{
			this.onClicked(null);
		}
	}

	function shouldDisplayTutorial()
	{
		return this.mForceShowTutorial || this.mDisplayCritera && this.mDisplayCritera.shouldDisplayTutorial();
	}

}

::_tutorialManager <- null;
