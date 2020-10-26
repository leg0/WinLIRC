function PluginViewModel(p) {
	var self = this;
	this.name = ko.observable(p);
}

function ConfigViewModel(c) {
	var self = this;
	this.name = ko.observable(c);
}

function SettingsViewModel() {
}


function AppViewModel() {
	var self = this;
	self.plugins = ko.observableArray();
	self.configs = ko.observableArray();
	//self.settings = ko.observable();

	$.getJSON("/api/v1/plugins", function(allData) {
        	var mappedPlugins = $.map(allData.plugins, function(item) { return new PluginViewModel(item) });
	        self.plugins(mappedPlugins);
	});  

	$.getJSON("/api/v1/configs", function(allData) {
        	var mappedConfigs = $.map(allData.configs, function(item) { return new ConfigViewModel(item) });
	        self.configs(mappedConfigs);
	});  
}

var appViewModel = new AppViewModel();
ko.applyBindings(appViewModel);

/*	$.get("/api/v1/configs", function (data, status) {
		appViewModel.configs(data.configs);
	});
	$.get("/api/v1/settings", function (data, status) {
		appViewModel.settings(data.settings);
	});*/
