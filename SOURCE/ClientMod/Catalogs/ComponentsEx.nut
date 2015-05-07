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
ComponentIndex["Item-IceEEBeta_Flag"] <- "Item-IceEEBeta_Flag";

ComponentIndex["CL-LitCandle1"] <- "Prop-ModAddons1";
ComponentIndex["CL-LitCandle2"] <- "Prop-ModAddons1";
ComponentIndex["CL-LitCandle3"] <- "Prop-ModAddons1";

ComponentIndex["Prop-Painting1-WantedShadow"] <- "Prop-ModWantedShadow";


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
