package templates

import (
	"fmt"
	"projectX.com/internal/store"
)

func GetSaveURL(device *store.Device) string {
	if device.ID == 0 {
		return "/ui/devices"
	}
	return fmt.Sprintf("/ui/devices/%d", device.ID)
}

func GetFormTitle(device *store.Device) string {
	if device.ID == 0 {
		return "Add New Device"
	}
	return "Edit Device"
}
