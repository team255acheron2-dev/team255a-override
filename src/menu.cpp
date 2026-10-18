#include "menu.hpp"

lv_obj_t* image;

int color; //Added in code somewhere. Can't harm us anymore
int textColor;
bool selectedIsBlue; //Update when storing. Used for driver motions and color detection

void drawImage() {
    LV_IMG_DECLARE(backgroundPhoto); //Declares the file

    //File Config stuff. Makes the photo
    image = lv_img_create(lv_scr_act());
    lv_img_set_src(image, &backgroundPhoto);
    lv_obj_set_size(image, 480, 240);
    lv_obj_align(image, LV_ALIGN_CENTER, 0, 0);
}