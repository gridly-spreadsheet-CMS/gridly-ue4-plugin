# Gridly for UE4

Gridly is the #1 headless CMS for Multilingual Game Development.

Manage your game’s data as a single source of truth and roll out continuous updates with full localization support & version control.

## Prerequisities

- Unreal Engine 4.26.0

## Getting Started

Before you use Gridly, you will need to sign up for an account at https://www.gridly.com

Download the Gridly plugin itself, as well as the sample project to get you started:

- [Gridly UE4 Plugin](https://bitbucket.org/localizedirect/gridly-ue4-plugin)
- [Gridly UE4 Sample Project](https://bitbucket.org/localizedirect/gridly-ue4-plugin-sample-project)

The sample project is optional, but provides a working example project for reference that has a very basic user interface set up for localization in UE4.

![Gridly UE4 Sample Project Screenshot](Documentation/SampleScreenshot.png)

### Setting up a Project for Localization

Gridly plugin for UE4 leverages Unreal's built-in tools for localization. If you are not familiar with Unreal's localization pipeline, please consult [their documentation](https://docs.unrealengine.com/en-US/ProductionPipelines/Localization/Overview/index.html).

This guide assumes that you are already familiar with Unreal's tools, and that you've correctly set up all the text in your project for localization using this workflow. The plugin supports both inline source strings and the use of StringTables.

### Installing the Plugin

The plugin can be installed as either an engine plugin or a project plugin.

#### As a Project Plugin

- Extract the contents of Gridly UE4 Plugin repository to `Plugins/Gridly` (relative to project root).
- Generate project files for the project and build the project to compile the required DLLs for the project plugin.

#### As an Engine Plugin

- Extract the contents of Gridly UE4 Plugin repository to `Engine/Plugins/Gridly` (relative to engine root).
- Generate project files for the engine and build again to compile the required DLLs for the plugin.

### Enabling the Plugin

To enable the plugin in your project, go to `Edit -> Plugins` and search for `Gridly`.

![Enable Gridly UE4 Plugin](Documentation/EnablePlugin.png)

Once enabled, restart Unreal to load the plugin.

### Changing the Localization Service Provider

In the Localization Dashboard, you should now be able to choose Gridly as your Localization Service Provider. Once selected, close the Localization Dashboard and re-open it to load the toolbar buttons.

![Choose Localizaton Service Provider](Documentation/ChooseLocalizationServiceProvider.png)

If you do not have access to the Localization Dashboard, you need to enable it. Go to `Edit -> Editor Preferences` and search for `Localization Dashboard` to activate it.

![Enable Localization Dashboard](Documentation/EnableLocalizationDashboard.png)

Once enabled, restart Unreal to be able to find the Localization Dashboard option under `Window -> Localization Dashboard`.

## Export/import to/from Gridly

Before you can export your source strings to Gridly, you need to first set up a new Grid in Gridly. Here is an example using [FIGS](https://en.wikipedia.org/wiki/FIGS):

![Create the Grid](Documentation/CreateGrid.png)

You will also need to set up your project to target the same cultures in the Localization Dashboard.

![Setting up Cultures](Documentation/Cultures.png)

Make sure these locale/culture codes match the ones on Gridly (hover over them to show tooltip). If you have previously set this up with codes that do not match those in Gridly, or you wish to map them in a different way, see how you can [customize the mapping](#markdown-header-custom-culture-mapping).

### Setting up the API keys and view IDs

To communicate with the API, you need to make sure that you have configured the plugin to use the correct API keys. You also need to set which view IDs you wish to use for import/export. See [import/export settings](#markdown-header-importexport-settings) for more information.

![API keys and view IDs](Documentation/GridlyAPISettings.png)

### Exporting Native Source Strings

Once you have made sure that your source and target languages are set up correctly and matching in both Unreal and on Gridly, and you have gathered all your source text, you can now export all source strings by pressing the `Export to Gridly`-button on the Localization Dashboard.

![Export to Gridly](Documentation/ExportGridly.png)

After export, open the Grid in your browser to edit the translations in the browser (may require refresh to see changes).

### Importing Translations

After you're done translating, you can import translations for all target cultures back to project with just a single click.

![Import from Gridly](Documentation/ImportGridly.png)

## Live Preview

The Gridly plugin also supports updating translations during runtime using the provided Blueprint functions to enable preview mode:

![Enable Live Preview Blueprint](Documentation/EnableLivePreviewBlueprint.png)

After making changes on Gridly, the translations can then be updated using the following:

![Update Live Preview Blueprint](Documentation/UpdateLivePreviewBlueprint.png)

While possible, it is currently *not* recommended to use this mode in a production build! This functionality is for development only (either in PIE mode or Development build). When final translations are ready, you should import your translations [through the Localization Dashboard](#markdown-header-importing-translations).

## Configuring Gridly

All the settings for Gridly can be found in `Edit -> Project Settings -> Plugins -> Gridly`. They can also be found in `Config/DefaultGame.ini` if you prefer to edit by hand.

### Import/export Settings

### Custom Culture Mapping
