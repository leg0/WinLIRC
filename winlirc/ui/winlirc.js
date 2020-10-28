var appViewModel;

function PluginViewModel(p) {
	var self = this;
	self.name = ko.observable(p.name);
	self.canRecord = ko.observable(p.canRecord);
	self.isActive = ko.computed(function() {
		return self == appViewModel.activePlugin();
	});

	self.activate = function() {
		appViewModel.activePlugin(self);
	};
}

function ConfigViewModel(cfg) {
	var self = this;
	self.name = ko.observable(cfg);
	self.isActive = ko.computed(function() {
		return self == appViewModel.activeConfig();
	});
	self.activate = function() {
		appViewModel.activeConfig(self);
	};
}

function SettingsViewModel() {
	var self = this;
}


function AppViewModel() {
	var self = this;
	self.plugins = ko.observableArray([]);
	self.configs = ko.observableArray([]);
	self.settings = ko.observable();
	self.activePlugin = ko.observable();
	self.activeConfig = ko.observable();
	self.version = "1.0";
	self.activePluginCanRecord = ko.computed(function() {
		var ap = self.activePlugin();
		return ap && ap.canRecord();
	});

	$.getJSON("/api/v1/plugins", function(allData) {
        	var mappedPlugins = $.map(allData.plugins, function(item) { return new PluginViewModel(item) });
	        self.plugins(mappedPlugins);
	});  

	$.getJSON("/api/v1/configs", function(allData) {
        	var mappedConfigs = $.map(allData.configs, function(item) { return new ConfigViewModel(item) });
	        self.configs(mappedConfigs);
	});

}

appViewModel = new AppViewModel();
ko.applyBindings(appViewModel);

/*	$.get("/api/v1/configs", function (data, status) {
		appViewModel.configs(data.configs);
	});
	$.get("/api/v1/settings", function (data, status) {
		appViewModel.settings(data.settings);
	});*/
