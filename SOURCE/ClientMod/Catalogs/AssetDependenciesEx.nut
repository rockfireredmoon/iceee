require("AssetDependencies");

//  Adds or modifies dependencies for CAR asset packages, to make sure other CAR
//  packages are loaded.


//  Fix a dependency issue that would prevent the exterior from loading.  (Typo between
//  "of" and "Of"
AssetDependencies["Bldg-Hall_Of_Bones"] <- ["Prop-BonesTex_A","Prop-Door","Prop-Crypt_Props"];





/* PF Additions */

AssetDependencies["Prop-ModAddons1"] <- ["Prop-Accessories1"];





/* 0.8.8 assets */

//  Bldg-Hall_Of_Bones1 doesn't seem to work in 0.8.6.  It appears to be a duplicate of
//  Bldg-Hall_Of_Bones (binary contents equivalent).  The archive contains the same file
//  list too.  Components.nut currently references Bldg-Hall_Of_Bones.

//  CL-Tent_Barricade requires an asset in Prop-Fences.car that doesn't exist in 0.8.6.

//AssetDependencies["Bldg-Hall_Of_Bones1"] <- ["Prop-BonesTex_A","Prop-Door","Prop-Crypt_Props"];

AssetDependencies["CL-Garden_Fence_BigSquare"] <- ["Prop-Fence-Stone"];
AssetDependencies["CL-Garden_Fence_Circle"] <- ["Prop-Fence-Stone"];
AssetDependencies["CL-Garden_Fence_Corner"] <- ["Prop-Fence-Stone"];
AssetDependencies["CL-Garden_Fence_Oval"] <- ["Prop-Fence-Stone"];
AssetDependencies["CL-Strange_Device"] <- ["Prop-Crystals", "Prop-Alchemy1"];
//AssetDependencies["CL-Tent_Barricade"] <- ["Prop-Tents1", "Prop-Fences"];


