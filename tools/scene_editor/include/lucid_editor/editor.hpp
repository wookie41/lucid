#pragma once

#include "scene/camera.hpp"
#include "scene/actors/actor.hpp"
#include "common/strings.hpp"
#include "common/types.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"

constexpr char EDITOR_WINDOW[]          = "Lucid Editor";
constexpr char DOCKSPACE_WINDOW[]       = "Dockspace";
constexpr char SCENE_VIEWPORT[]         = "Scene";
constexpr char RESOURCES_BROWSER[]      = "Resources browser";
constexpr char ASSETS_BROWSER[]         = "Assets browser";
constexpr char SCENE_HIERARCHY[]        = "Scene hierarchy";
constexpr char MAIN_DOCKSPACE[]         = "MainDockSpace";
constexpr char POPUP_WINDOW[]           = "PopupWindow";
constexpr char ACTOR_DETAILS[]          = "Actor Details";
constexpr char COMMON_ACTORS[]          = "Common actors";
constexpr char ACTOR_ASSET_DRAG_TYPE[]  = "ACTOR_ASSET_DRAG_TYPE";
constexpr char COMMON_ACTOR_DRAG_TYPE[] = "COMMON_ACTOR_DRAG_TYPE";

namespace lucid
{
    enum class EImportingTextureType : u8
    {
        JPG,
        PNG
    };

    enum class ECommonActorType : u8
    {
        DIRECTIONAL_LIGHT,
        SPOT_LIGHT,
        POINT_LIGHT
    };

    namespace platform
    {
        class CWindow;
    };

    namespace resources
    {
        class CMeshResource;
        class CTextureResource;
    }; // namespace resources

    namespace scene
    {
        class CWorld;
        class IActor;
    }; // namespace scene

    namespace gpu
    {
        class CShader;
    };

    struct FSceneEditorState
    {
        platform::CWindow* Window;
        ImGuiContext*      ImGuiContext;
        ImGuiWindow* ImSceneWindow;

        ImGuiID MainDockId  = 0;
        ImGuiID SceneDockId = 0;

        u16   EditorWindowWidth       = 1920;
        u16   EditorWindowHeight      = 1080;
        float EditorWindowAspectRatio = 1280.f / 720.f;

        /** These are updated when drawing ui */
        float  SceneWindowWidth;
        float  SceneWindowHeight;
        ImVec2 SceneWindowPos;

        /** User interface stuff */
        bool bIsAnyUIWidgetHovered = false;
        bool bIsFileDialogOpen     = false;

        scene::CWorld* World              = nullptr;
        scene::CWorld* PendingDeleteWorld = nullptr;

        scene::IActor* CurrentlySelectedActor = nullptr;
        scene::IActor* LastDeletedActor       = nullptr;
        scene::IActor* ClipboardActor         = nullptr;

        scene::CCamera  PerspectiveCamera{ scene::ECameraMode::PERSPECTIVE };
        scene::CCamera* CurrentCamera = nullptr;

        /** File dialog related things, used e.x. when importing assets */
        bool               bShowFileDialog = false;
        ImGui::FileBrowser FileDialog;
        void (*OnFileSelected)(const std::filesystem::path&);

        /** Variables used when importing an asset to the engine */

        char                  AssetNameBuffer[256];
        FDString              PathToSelectedFile{ "" };
        EImportingTextureType ImportingTextureType;

        bool bAssetNameMissing       = false;
        bool bIsImportingMesh        = false;
        bool bIsImportingTexture     = false;
        bool bFailedToImportResource = false;

        resources::CMeshResource*    ClickedMeshResource    = nullptr;
        resources::CTextureResource* ClickedTextureResource = nullptr;
        scene::CMaterial*            ClickedMaterialAsset   = nullptr;
        scene::IActor*               ClickedActorAsset      = nullptr;

        bool bDisableCameraMovement = false;

        bool bShowMaterialAssets = true;
        bool bShowActorAssets    = false;

        scene::EMaterialType TypeOfMaterialToCreate = scene::EMaterialType::NONE;
        gpu::CShader*        PickedShader           = nullptr;
        scene::CMaterial*    EditedMaterial         = nullptr;

        scene::EActorType TypeOfActorToCreate = scene::EActorType::UNKNOWN;
        scene::IActor*    EditedActorAsset    = nullptr;

        bool GenericBoolParam0 = false;
        bool GenericBoolParam1 = false;

        float GenericFloat2Param0[2] = { 0 };
        float GenericFloat2Param1[2] = { 0 };

        int GenericIntParam0 = 0;
        int GenericIntParam1 = 0;

        float GenericFloatParam0 = 0;
        float GenericFloatParam1 = 0;
        float GenericFloatParam2 = 0;
        float GenericFloatParam3 = 0;
        float GenericFloatParam4 = 0;
        float GenericFloatParam5 = 0;

        int ResourceItemsPerRow = 12;

        bool bIsRunning = true;

        float LastActorSpawnTime = 0;

        FDString WorldFilePath;

        bool bSavingWorldToFile = false;
        bool bCreatingNewWorld  = false;

        static constexpr int NumVideoMemoryUsageSamples                   = 60 * 5;
        float                VideoMemoryUsage[NumVideoMemoryUsageSamples] = { 0 };
        int                  VideoMemoryUsageIndex                        = 0;
        float                SecondsSinceLastVideoMemorySnapshot          = 0;

        static constexpr int NumDrawCallSamples               = 60 * 5;
        float                NumDrawCalls[NumDrawCallSamples] = { 0 };
        int                  NumDrawCallsIndex                = 0;

        static constexpr int NumFrameTimesSamples             = 60 * 5;
        float                FrameTimes[NumFrameTimesSamples] = { 0 };
        int                  FrameTimesIndex                  = 0;

        bool bShowingControlsWindow        = false;
        bool bShowingStatsWindow           = false;
        bool bShowingRendererSettinsWindow = false;

        bool bBlockActorPicking = false;
    };

    extern FSceneEditorState GSceneEditorState;
} // namespace lucid
