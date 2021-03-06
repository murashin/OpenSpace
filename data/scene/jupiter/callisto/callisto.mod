return {
    -- RenderableGlobe module
    {
        Name = "Callisto",
        Parent = "JupiterBarycenter",
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_CALLISTO",
                DestinationFrame = "GALACTIC",
            },
            Translation = {
                Type = "SpiceTranslation",
                Target = "CALLISTO",
                Observer = "JUPITER BARYCENTER",
                Kernels = "${OPENSPACE_DATA}/spice/jup260.bsp"
            }
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = 2410000,
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "Callisto Texture",
                        FilePath = "textures/callisto.jpg",
                        Enabled = true
                    }
                }
            }
        },
        GuiPath = "/Solar System/Planets/Jupiter/Moons"
    },

    -- Trail module
    {   
        Name = "CallistoTrail",
        Parent = "JupiterBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Target = "CALLISTO",
                Observer = "JUPITER BARYCENTER"
            },
            Color = { 0.4, 0.3, 0.01 },
            Period = 17,
            Resolution = 1000
        },
        GuiPath = "/Solar System/Planets/Jupiter/Moons"
    }
}
