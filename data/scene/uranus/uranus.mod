return {
    -- Barycenter module
    {
        Name = "UranusBarycenter",
        Parent = "SolarSystemBarycenter",
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Target = "URANUS BARYCENTER",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            }
        },
        GuiPath = "/Solar System/Planets/Uranus"
    },
    -- RenderableGlobe module
    {   
        Name = "Uranus",
        Parent = "UranusBarycenter",
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_URANUS",
                DestinationFrame = "GALACTIC"
            }
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = { 25559000, 25559000, 24973000 },
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Texture",
                        FilePath = "textures/uranus.jpg",
                        Enabled = true
                    }
                }
            }
        },
        Tag = { "planet_solarSystem", "planet_giants" },
        GuiPath = "/Solar System/Planets/Uranus"
    },
    -- Trail module
    {   
        Name = "UranusTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "URANUS BARYCENTER",
                Observer = "SUN"
            },
            Color = {0.60, 0.95, 1.00 },
            Period = 30588.740,
            Resolution = 1000
        },
        Tag = { "planetTrail_solarSystem", "planetTrail_giants" },
        GuiPath = "/Solar System/Planets/Uranus"
    }
}
