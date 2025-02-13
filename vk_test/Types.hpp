union QueueFamilyIndexMap {
  struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
  };

  uint32_t families[2];
};
