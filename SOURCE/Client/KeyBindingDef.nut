this.KeyBindingDef <- {};
function c_KB( kc )
{
	return "Ctrl+" + this.Key.getText(kc);
}

function a_KB( kc )
{
	return "Alt+" + this.Key.getText(kc);
}

function s_KB( kc )
{
	return "Shift+" + this.Key.getText(kc);
}

function ca_KB( kc )
{
	return "Ctrl+Alt+" + this.Key.getText(kc);
}

function C_KB( kc )
{
	return "Ctrl+Shift+" + this.Key.getText(kc);
}

function A_KB( kc )
{
	return "Alt+Shift+" + this.Key.getText(kc);
}

function CSA_KB( kc )
{
	return "Ctrl+Alt+Shift+" + this.Key.getText(kc);
}

function _KB( kc )
{
	return "^" + this.Key.getText(kc);
}

function KB( kc )
{
	return this.Key.getText(kc);
}

this.KeyBindingDef.Load <- function ()
{
	this.KeyBindingDef[this.KB(this.Key.VK_ENTER)] <- "onStartChatInput";
	this.KeyBindingDef[this.KB(this.Key.VK_DIVIDE)] <- "onStartCommandInput";
	this.KeyBindingDef[this.KB(191)] <- "onStartCommandInput";
	this.KeyBindingDef[this.c_KB(this.Key.VK_C)] <- "onChatWindowToggle";
	this.KeyBindingDef[this.s_KB(this.Key.VK_C)] <- "onChatListVisibleToggle";
	this.KeyBindingDef[this.c_KB(this.Key.VK_I)] <- "/inventory";
	this.KeyBindingDef[this.c_KB(this.Key.VK_W)] <- "/togglePolygonMode";
	this.KeyBindingDef[this.s_KB(this.Key.VK_F1)] <- "";
	this.KeyBindingDef[this.s_KB(this.Key.VK_F2)] <- "";
	this.KeyBindingDef[this.s_KB(this.Key.VK_F3)] <- "/toggleBuilding";
	this.KeyBindingDef[this.s_KB(this.Key.VK_F4)] <- "/creatureTweak";
	this.KeyBindingDef[this.s_KB(this.Key.VK_F5)] <- "/creatureBrowse";
	this.KeyBindingDef[this.KB(this.Key.VK_F6)] <- "/importexcel";
	this.KeyBindingDef[this.KB(this.Key.VK_F7)] <- "/togglePreview";
	this.KeyBindingDef[this.KB(this.Key.VK_F9)] <- "onDebugWindowToggle";
	this.KeyBindingDef[this.KB(this.Key.VK_F10)] <- "/configVideo";
	this.KeyBindingDef[this.a_KB(this.Key.VK_Z)] <- "/toggleUI";
	this.KeyBindingDef[this.KB(this.Key.VK_X)] <- "/toggleWeapons";
};
