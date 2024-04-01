components {
  id: "Logic"
  component: "/Pragma/Options.script"
  position {
    x: 0.0
    y: 0.0
    z: 0.0
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
}
embedded_components {
  id: "Panel"
  type: "sprite"
  data: "default_animation: \"Hint\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "slice9 {\n"
  "  x: 15.0\n"
  "  y: 15.0\n"
  "  z: 15.0\n"
  "  w: 15.0\n"
  "}\n"
  "size {\n"
  "  x: 1276.0\n"
  "  y: 2086.0\n"
  "  z: 0.0\n"
  "  w: 0.0\n"
  "}\n"
  "size_mode: SIZE_MODE_MANUAL\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/UI.atlas\"\n"
  "}\n"
  ""
  position {
    x: 337.5
    y: 540.0
    z: 0.992
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "Titles"
  type: "sprite"
  data: "default_animation: \"OptionText\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 337.5
    y: 540.0
    z: 0.993
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "OffsetTypeButton"
  type: "sprite"
  data: "default_animation: \"OptionDirect\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsB.atlas\"\n"
  "}\n"
  ""
  position {
    x: 172.5
    y: 594.0
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "AudioLatencyButton"
  type: "sprite"
  data: "default_animation: \"OptionOffset\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 468.5
    y: 644.75
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "InputDeltaButton"
  type: "sprite"
  data: "default_animation: \"OptionOffset\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 468.5
    y: 543.25
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "AudioLatencyText"
  type: "label"
  data: "size {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 0.0\n"
  "}\n"
  "color {\n"
  "  x: 1.0\n"
  "  y: 1.0\n"
  "  z: 1.0\n"
  "  w: 0.94\n"
  "}\n"
  "outline {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "shadow {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "leading: 0.0\n"
  "tracking: 0.0\n"
  "pivot: PIVOT_CENTER\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "line_break: false\n"
  "text: \"0 ms\"\n"
  "font: \"/System/Ar.font\"\n"
  "material: \"/builtins/fonts/label.material\"\n"
  ""
  position {
    x: 468.5
    y: 644.75
    z: 0.995
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "InputDeltaText"
  type: "label"
  data: "size {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 0.0\n"
  "}\n"
  "color {\n"
  "  x: 1.0\n"
  "  y: 1.0\n"
  "  z: 1.0\n"
  "  w: 0.94\n"
  "}\n"
  "outline {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "shadow {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "leading: 0.0\n"
  "tracking: 0.0\n"
  "pivot: PIVOT_CENTER\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "line_break: false\n"
  "text: \"0 ms\"\n"
  "font: \"/System/Ar.font\"\n"
  "material: \"/builtins/fonts/label.material\"\n"
  ""
  position {
    x: 468.5
    y: 543.25
    z: 0.995
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "HapticToggle"
  type: "sprite"
  data: "default_animation: \"OptionHaptic\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 214.75
    y: 370.5
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "HitSoundToggle"
  type: "sprite"
  data: "default_animation: \"OptionHitSound\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 467.25
    y: 370.5
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "ImportButton"
  type: "sprite"
  data: "default_animation: \"OptionImport\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 172.5
    y: 172.0
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "ExportButton"
  type: "sprite"
  data: "default_animation: \"OptionExport\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 341.0
    y: 172.0
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "CreditButton"
  type: "sprite"
  data: "default_animation: \"OptionCredit\"\n"
  "material: \"/builtins/materials/sprite.material\"\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "textures {\n"
  "  sampler: \"texture_sampler\"\n"
  "  texture: \"/Atlas/OptionsA.atlas\"\n"
  "}\n"
  ""
  position {
    x: 509.5
    y: 172.0
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
embedded_components {
  id: "Message"
  type: "label"
  data: "size {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 0.0\n"
  "}\n"
  "color {\n"
  "  x: 1.0\n"
  "  y: 1.0\n"
  "  z: 1.0\n"
  "  w: 0.64\n"
  "}\n"
  "outline {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "shadow {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "leading: 0.0\n"
  "tracking: 0.0\n"
  "pivot: PIVOT_SW\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "line_break: false\n"
  "text: \"0 fps\"\n"
  "font: \"/System/Ar.font\"\n"
  "material: \"/builtins/fonts/label.material\"\n"
  ""
  position {
    x: 37.0
    y: 37.0
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.4185
    y: 0.4185
    z: 1.0
  }
}
embedded_components {
  id: "Exit"
  type: "label"
  data: "size {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 0.0\n"
  "}\n"
  "color {\n"
  "  x: 1.0\n"
  "  y: 1.0\n"
  "  z: 1.0\n"
  "  w: 0.94\n"
  "}\n"
  "outline {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "shadow {\n"
  "  x: 0.0\n"
  "  y: 0.0\n"
  "  z: 0.0\n"
  "  w: 1.0\n"
  "}\n"
  "leading: 0.0\n"
  "tracking: 0.0\n"
  "pivot: PIVOT_NE\n"
  "blend_mode: BLEND_MODE_ALPHA\n"
  "line_break: false\n"
  "text: \"Exit\\343\\200\\214\\303\\227\\343\\200\\215\"\n"
  "font: \"/System/Ar.font\"\n"
  "material: \"/builtins/fonts/label.material\"\n"
  ""
  position {
    x: 638.0
    y: 1043.0
    z: 0.994
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale {
    x: 0.5
    y: 0.5
    z: 1.0
  }
}
