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

function AppViewModel() {
	var self = this;
	self.plugins = ko.observableArray([]);
	self.configs = ko.observableArray([]);
	self.activePlugin = ko.observable();
	self.activeConfig = ko.observable();
	self.disableAllRepeats = ko.observable(false);
	self.disableFirstNRepeats = ko.observable(0);
	self.enableLocalConnectionsOnly = ko.observable(true);
	self.serverPort = ko.observable(1111);
	self.enableTray = ko.observable(true);
	self.enableAutoStart = ko.observable(false);

	self.version = "1.0";
	self.activePluginCanRecord = ko.computed(function() {
		var ap = self.activePlugin();
		return ap && ap.canRecord();
	});

	self.activatePluginByName = function(pluginName) {
		var pl = self.plugins().find(function(x) { return x.name() == pluginName});
		if (pl)
			self.activePlugin(pl);
		else
			self.activePluginName = pluginName;
	};

	self.activateConfigByName = function(configName) {
		var cf = self.configs().find(function(x) { return x.name() == configName});
		if (cf) {
			self.activeConfig(cf);
		}
		else {
			self.activeConfigName = configName;
		}
	};

	self.saveConfig = function() {
		var d = { 
				"settings": {
					"plugin": self.activePlugin().name(),
					"config": self.activeConfig().name(),
					"disable_all_repeats": self.disableAllRepeats(),
					"disable_first_n_repeats": self.disableFirstNRepeats(),
					"enable_local_connections_only": self.enableLocalConnectionsOnly(),
					"server_port": self.serverPort(),
					"enable_tray": self.enableTray(),
					"enable_autostart": self.enableAutoStart()
				}
			};
		$.ajax({
			url: "/api/v1/settings",
			method: "PUT",
			mimeType: 'text/json',
			data: JSON.stringify(d)
		}).done(function() {
			alert('done');
		}).fail(function() {
			alert('error');
		});
	};

	$.getJSON("/api/v1/plugins", function(allData) {
        	var mappedPlugins = $.map(allData.plugins, function(item) { return new PluginViewModel(item) });
	        self.plugins(mappedPlugins);
			if (self.activePluginName) {
				self.activatePluginByName(self.activePluginName);
				self.activePluginName = undefined;
			}
	});  

	$.getJSON("/api/v1/configs", function(allData) {
        	var mappedConfigs = $.map(allData.configs, function(item) { return new ConfigViewModel(item) });
	        self.configs(mappedConfigs);
			if (self.activeConfigName) {
				self.activateConfigByName(self.activeConfigName);
				self.activeConfigName = undefined;
			}
	});

	$.getJSON("/api/v1/settings", function(allData) {
		var settings = allData.settings;
		console.log("Got settings", allData);
		if ('plugin' in settings)
			self.activatePluginByName(settings.plugin);
		if ('config' in settings)
			self.activateConfigByName(settings.config);
		if ('disable_all_repeats' in settings)
			self.disableAllRepeats(settings.disable_all_repeats);
		if ('disable_first_n_repeats' in settings)
			self.disableFirstNRepeats(settings.disable_first_n_repeats);
		if ('enable_local_connections_only' in settings)
			self.enableLocalConnectionsOnly(settings.enable_local_connections_only);
		if ('server_port' in settings)
			self.serverPort(settings.server_port)
		if ('enable_tray' in settings)
			self.enableTray(settings.enable_tray);
		if ('enable_autostart' in settings)
			self.enableAutoStart(settings.enable_autostart);
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
