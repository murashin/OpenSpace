earthEllipsoid = {6378137.0, 6378137.0, 6356752.314245} -- Earth's radii
return {
    -- Earth barycenter module
    {
        Name = "EarthBarycenter",
        Parent = "SolarSystemBarycenter",
        Transform = {
            Translation = {
                Type = "SpiceTranslation",
                Body = "EARTH",
                Observer = "SUN",
                Kernels = "${OPENSPACE_DATA}/spice/de430_1850-2150.bsp"
            },
        },
    },
    -- EarthTrail module
    {   
        Name = "EarthTrail",
        Parent = "SolarSystemBarycenter",
        Renderable = {
            Type = "RenderableTrailOrbit",
            Translation = {
                Type = "SpiceTranslation",
                Body = "EARTH",
                Observer = "SUN"
            },
            Color = { 0.5, 0.8, 1.0 },
            -- StartTime = "2016 JUN 01 12:00:00.000",
            -- EndTime = "2017 JAN 01 12:00:00.000",
            -- SampleInterval = 3600
            Period = 365.242,
            Resolution = 1000
        },
        GuiName = "/Solar/EarthTrail"
    },
    -- RenderableGlobe module
    {
        Name = "Earth",
        Parent = "EarthBarycenter",
        Transform = {
            Rotation = {
                Type = "SpiceRotation",
                SourceFrame = "IAU_EARTH",
                DestinationFrame = "GALACTIC",
            },
            Scale = {
                Type = "StaticScale",
                Scale = 1,
            },
        },
        Renderable = {
            Type = "RenderableGlobe",
            Radii = earthEllipsoid,
            CameraMinHeight = 300,
            InteractionDepthBelowEllipsoid = 0, -- Useful when having negative height map values
            SegmentsPerPatch = 64,
            Layers = {
                ColorLayers = {
                    {
                        Name = "ESRI VIIRS Combo",
                        Type = "ByLevel",
                        LevelTileProviders = {
                            {
                                MaxLevel = 7, 
                                TileProvider = { FilePath = "map_service_configs/GIBS/VIIRS_SNPP_CorrectedReflectance_TrueColor.xml", }, 
                            },
                            {
                                MaxLevel = 22, 
                                TileProvider = { FilePath = "map_service_configs/ESRI/ESRI_Imagery_World_2D.wms" },
                            },
                        },
                        Enabled = true,
                    },
                    --[[
                    {
                        Name = "TestByIndex",
                        Type = "ByIndex",
                        IndexTileProviders = {
                            {
                                TileIndex = { Level = 2, X = 0, Y = 0},
                                TileProvider = {
                                    Type = "SingleImage",
                                    FilePath = "../../debugglobe/textures/test_tile.png",
                                },
                            },
                            {
                                TileIndex = { Level = 4, X = 2, Y = 3},
                                TileProvider = {
                                    Type = "SingleImage",
                                    FilePath = "../../debugglobe/textures/test_tile.png",
                                },
                            },
                            {
                                TileIndex = { Level = 6, X = 8, Y = 12},
                                TileProvider = {
                                    Type = "SingleImage",
                                    FilePath = "../../debugglobe/textures/earth_bluemarble.jpg",
                                },
                            },
                        },
                        DefaultProvider = {
                            FilePath = "map_service_configs/ESRI/ESRI_Imagery_World_2D.wms",
                        },
                    },
                    ]]
                    {
                        Type = "Temporal",
                        Name = "Temporal VIIRS SNPP",
                        FilePath = "map_service_configs/GIBS/Temporal_VIIRS_SNPP_CorrectedReflectance_TrueColor.xml",
                    },
                    {
                        Type = "Temporal",
                        Name = "Temporal_GHRSST_L4_MUR_Sea_Surface_Temperature",
                        FilePath = "map_service_configs/GIBS/Temporal_GHRSST_L4_MUR_Sea_Surface_Temperature.xml",
                    },
                    {
                        Type = "SingleImage",
                        Name = "Debug Tiles",
                        FilePath = "../../debugglobe/textures/test_tile.png",
                    },
                },
                GrayScaleLayers = { },
                GrayScaleColorOverlays = { },
                NightLayers = {
                    {
                        Name = "Earth at Night 2012",
                        FilePath = "map_service_configs/GIBS/VIIRS_CityLights_2012.xml",
                        Enabled = true,
                    },
                },
                WaterMasks = {
                    {
                        Name = "MODIS_Water_Mask",
                        FilePath = "map_service_configs/GIBS/MODIS_Water_Mask.xml",
                        Enabled = true,
                    },
                },
                ColorOverlays = {
                    {
                        Name = "Coastlines",
                        FilePath = "map_service_configs/GIBS/Coastlines.xml",
                    },
                    {
                        Name = "Reference_Features",
                        FilePath = "map_service_configs/GIBS/Reference_Features.xml",
                    },
                    {
                        Name = "Reference_Labels",
                        FilePath = "map_service_configs/GIBS/Reference_Labels.xml",
                    },
                    {
                        Type = "TileIndex",
                        Name = "Tile Indices",
                    },
                    {
                        Type = "SizeReference",
                        Name = "Size Reference",
                        Radii = earthEllipsoid,
                        BackgroundImagePath = "../../debugglobe/textures/arrows.png",
                    },
                    
                    {
                        Name = "Presentation",
                        Type = "PresentationSlides",
                        TileIndex = { Level = 11, X = 1649, Y = 511},
                        SlideProviders = {
                            { Type = "SingleImage", FilePath = "../../presentation/01.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/02.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/03.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/04.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/05.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/06.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/07.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/08.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/09.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/10.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/11.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/12.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/13.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/14.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/15.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/16.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/17.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/18.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/19.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/20.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/21.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/22.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/23.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/24.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/25.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/26.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/27.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/28.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/29.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/30.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/31.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/32.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/33.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/34.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/35.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/36.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/37.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/38.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/39.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/40.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/41.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/42.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/43.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/44.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/45.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/46.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/47.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/48.png",},
                            { Type = "SingleImage", FilePath = "../../presentation/49.png",},
                        },
                        DefaultProvider = { Type = "SingleImage", FilePath = "../../debugglobe/textures/black.png", },
                    },
                },
                HeightLayers = {
                    {
                        Name = "Terrain tileset",
                        FilePath = "map_service_configs/ESRI/TERRAIN.wms",
                        Enabled = true,
                        MinimumPixelSize = 64,
                        DoPreProcessing = true,
                    },
                },
            },
        }
    },
}