#pragma once

struct zmap_header_type;
struct gdrv_bitmap8;
struct render_sprite_type_struct;
struct component_control;
struct vector2;
class TPinballTable;

enum class message_code
{
	Reset = 1024,
	LightActiveCount = 37,
	LightTotalCount = 38,
	LightSetMessageField = 23,
};

class TPinballComponent
{
public:
	TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals);
	virtual ~TPinballComponent();
	virtual int Message(int code, float value);
	virtual void port_draw();
	int get_scoring(unsigned int index) const;
	virtual vector2 get_coordinates();

	char UnusedBaseFlag;
	char ActiveFlag;
	int MessageField;
	char* GroupName;
	component_control* Control;
	int GroupIndex;
	render_sprite_type_struct* RenderSprite;
	TPinballTable* PinballTable;
	std::vector<gdrv_bitmap8*>* ListBitmap;
	std::vector<zmap_header_type*>* ListZMap;
private:
	float VisualPosNormX;
	float VisualPosNormY;
};
