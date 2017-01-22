/* Merge elements into the existing table. */

//  Adds or modifies components (scenery entities, typically .csm.xml files)
//  and maps them to the CAR asset packages they reside in.  Scenery cannot be
//  loaded unless a component is defined.

require("Components");

/* PF addition */
ComponentIndex["Prop-Endtable2"] <- "Prop-ModAddons1";
ComponentIndex["Prop-Endtable3"] <- "Prop-ModAddons1";
ComponentIndex["Par-Snow3-Emitter"] <- "Prop-ModAddons1";
ComponentIndex["Prop-Painting2"] <- "Prop-ModAddons1";
ComponentIndex["Prop-Painting2a"] <- "Prop-ModAddons1";
ComponentIndex["Prop-Painting3a"] <- "Prop-ModAddons1";
ComponentIndex["Prop-Rug_Teal"] <- "Prop-ModAddons1";

ComponentIndex["CL-LitCandle1"] <- "Prop-ModAddons1";
ComponentIndex["CL-LitCandle2"] <- "Prop-ModAddons1";
ComponentIndex["CL-LitCandle3"] <- "Prop-ModAddons1";
ComponentIndex["CL-Candelabra2"] <- "";

ComponentIndex["Prop-Painting1-WantedShadow"] <- "Prop-ModWantedShadow";

// IceEE additions

ComponentIndex["Prop-Crystal_Black_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Black_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Colourful_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Colourful_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Cyan_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Cyan_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Gold_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Gold_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Orange_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Orange_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Red_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Red_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_RaisedPurples_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_RaisedPurples_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Violet_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Violet_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_White_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_White_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-Crystal_Yellow_Huge1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Huge2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Huge3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Med1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Med2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Med3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Med4"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Med5"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Pipe1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Pipe2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Pipe3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Small1"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Small2"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Small3"] <- "Prop-Crystals";
ComponentIndex["Prop-Crystal_Yellow_Small4"] <- "Prop-Crystals";

ComponentIndex["Prop-TNT"] <- "Prop-Containers2";

ComponentIndex["Prop-Wooden_Barricade_Sm1_far"] <- "Prop-Fences";
ComponentIndex["CL-Farm-Chicken_Coop1"] <- "CL-Farm";

ComponentIndex["Par-BigExplosion"] <- "Effects";
ComponentIndex["Par-Flame_Green-Emitter"] <- "Effects";
ComponentIndex["Par-Flame_Red-Emitter"] <- "Effects";

ComponentIndex["Item-IceEESage_Crystal"] <- "Item-IceEESage_Crystal";
ComponentIndex["Item-IceEEBeta_Flag"] <- "Item-IceEEBeta_Flag";

ComponentIndex["Prop-CTF_Red"] <- "Prop-CTF";
ComponentIndex["Prop-CTF_Blue"] <- "Prop-CTF";

/* Copied from EER */

ComponentIndex["Prop-Battlefield_Stone2"] <- "Prop-Battlefield1";
ComponentIndex["Prop-Clutter-crRock1"] <- "Prop-Clutter1";
ComponentIndex["Prop-Clutter-crRock2"] <- "Prop-Clutter1";
ComponentIndex["Prop-Clutter-crRock3"] <- "Prop-Clutter1";
ComponentIndex["Prop-Mother_Shard"] <- "Prop-Crystals";
ComponentIndex["Prop-Fence_Sticks1"] <- "Prop-Fences";
ComponentIndex["Prop-Fence_Sticks2"] <- "Prop-Fences";
ComponentIndex["Prop-Garden_Raised_Bed_Round"] <- "Prop-Gardens";
ComponentIndex["Prop-Tree_Platform_Bridge_Endcap"] <- "Prop-Tree_Platforms";
ComponentIndex["Prop-Archery_Platform"] <- "Prop-Walls";


/* 0.8.8+ */

/*  Bldg-Hall_Of_Bones1 doesn't seem to work in 0.8.6.
    CL-Tent_Barricade requires an asset in Prop-Fences.car that doesn't exist in 0.8.6.
*/

//ComponentIndex["Bldg-Hall_Of_Bones1"] <- "Bldg-Hall_Of_Bones1";
ComponentIndex["CL-Garden_Fence_BigSquare"] <- "CL-Garden_Fence_BigSquare";
ComponentIndex["CL-Garden_Fence_Circle"] <- "CL-Garden_Fence_Circle";
ComponentIndex["CL-Garden_Fence_Corner"] <- "CL-Garden_Fence_Corner";
ComponentIndex["CL-Garden_Fence_Oval"] <- "CL-Garden_Fence_Oval";
ComponentIndex["CL-Strange_Device"] <- "CL-Strange_Device";
//ComponentIndex["CL-Tent_Barricade"] <- "CL-Tent_Barricade";    //** needs fix
ComponentIndex["Prop-Backdrop"] <- "Prop-Backdrop";
ComponentIndex["Prop-Desert_Dolmen"] <- "Prop-Desert_Dolmen";
ComponentIndex["Prop-Cage"] <- "Prop-Prison_Items";
ComponentIndex["Prop-Tent_Earthrise"] <-"Prop-Tent_Earthrise";
