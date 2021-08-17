const uri_wirbelwind_box = "" // "http://192.168.4.1" //" "http://wirbelwind.box" //  
const headers = { "Content-Type": "application/json" }

const { createApp, reactive } = Vue

const settings = createApp({
	data() {
		return {
			wifis: [],
			password: '',
			bluetooth_state: false,
			wifi_task_state: false,
			toggleWifiView: false,
			new_firmware_version: '',
			new_website_version: '',
			current_firmware_version: '',
			current_website_version: '',
			toggleUpdateView: false,
		}
	},
	created() {
		axios.get(uri_wirbelwind_box + "/networks?enable_wifi_task", { headers })
			.then(response => this.wifi_task_state = response.data.enable_wifi_task)
			.catch(error => {
				this.errorMessage = error.message;
				console.error("There was an error!", error);
			})
	},
	watch: {
		bluetooth_state(state) {
			alert("Diese Funktion ist aktuell auf Grund von Speichermangel noch nicht verfügbar :-)");
		},
		wifi_task_state(state) {
			axios.get(uri_wirbelwind_box + "/networks?enable_wifi_task=" + state, { headers })
				.then(response => this.wifi_task_state = response.data.enable_wifi_task)
				.catch(error => {
					this.errorMessage = error.message;
					console.error("There was an error!", error);
				})
		}
	},
	computed: {

	},
	methods: {
		addNetwork: async function (event, ssid) {
			const response = await axios.post(uri_wirbelwind_box + "/networks",
				{
					SSID: ssid,
					PWD: event.target.value
				});
		},
		toggleWifi() {
			if (!this.toggleWifiView) {
				console.log("Scanning...");
				axios.get(uri_wirbelwind_box + "/networks?list=active", { headers })
					.then(response => this.wifis = response.data.list_active_wifis)
					.catch(error => {
						this.errorMessage = error.message;
						console.error("There was an error!", error);
					})
			}
			this.toggleWifiView = !this.toggleWifiView;
		},
		async toggleUpdates() {
			if (!this.toggleUpdateView) {
				const response = await axios.get(uri_wirbelwind_box + "/update?refresh", { headers });
				this.new_firmware_version = response.data.new_firmware_version;
				this.new_website_version = response.data.new_website_version;
				this.current_firmware_version = response.data.current_firmware_version;
				this.current_website_version = response.data.current_website_version;
			}
			this.toggleUpdateView = !this.toggleUpdateView;
		},
		updateFirmware() {
			axios.get(uri_wirbelwind_box + "/update?firmware", { headers });
		},
		updateWebsite() {
			axios.get(uri_wirbelwind_box + "/update?website", { headers });
		}
	}
}).mount('#app-settings')

const reactive_playlists = reactive({
	data() {
		return {
			playlists: []
		}
	},
	update() {
		axios.get(uri_wirbelwind_box + "/playlist", { headers })
			.then(response => this.playlists = response.data)
			.catch(error => {
				this.errorMessage = error.message;
				console.error("There was an error!", error);
			})
	}
})

const reactive_active_playlist = reactive({
	data() {
		return {
			uuid: -1,
			curr_track: -1,
			curr_timestamp: -1,
			curr_max_timestamp: -1
		}
	}
})

const reactive_selected_playlist = reactive({
	data() {
		return {
			uuid: '',
			name: '',
			volume: 0,
			curr_track: 0,
			curr_timestamp: 0,
			curr_max_timestamp: 0,
			tracks: []
		}
	},
	async refresh() {
		const response = await axios.post(uri_wirbelwind_box + "/playlist", { uuid: this.uuid });
		this.uuid = response.data[0].uuid;
		this.name = response.data[0].name;
		this.tracks = response.data[0].tracks;
		this.volume = response.data[1].volume;
		this.curr_track = response.data[1].curr_track;
		this.curr_timestamp = response.data[1].curr_timestamp;
		this.curr_max_timestamp = response.data[1].curr_max_timestamp;
	},
	async updateName(name) {
		const response = await axios.post(uri_wirbelwind_box + "/playlist",
			{
				uuid: this.uuid,
				name: name
			});
		this.name = response.data[0].name;
	},
	async updateCurrTimestamp() {
		const response = await axios.post(uri_wirbelwind_box + "/playlist",
			{
				uuid: this.uuid,
				curr_timestamp: this.curr_timestamp
			});
		this.curr_timestamp = response.data[1].curr_timestamp;
	},
	async updateCurrTrack(index) {
		const response = await axios.post(uri_wirbelwind_box + "/playlist",
			{
				uuid: this.uuid,
				curr_track: index,
				curr_timestamp: 0
			});
		this.curr_track = response.data[1].curr_track;
		this.curr_timestamp = response.data[1].curr_timestamp;
		this.curr_max_timestamp = response.data[1].curr_max_timestamp;
	},
	async updateVolume() {
		await axios.post(uri_wirbelwind_box + "/playlist",
			{
				uuid: this.uuid,
				volume: this.volume
			})
			.then(response => this.volume = response.data[1].volume)
			.catch(error => {
				this.errorMessage = error.message;
				console.error("There was an error!", error);
			});
	},
	async addTracks(path) {
		await axios.post(uri_wirbelwind_box + "/playlist",
			{
				uuid: this.uuid,
				add: [path]
			})
			.then(response => this.tracks = response.data[0].tracks)
			.catch(error => {
				this.errorMessage = error.message;
				console.error("There was an error!", error);
			});
	},
	async deleteTracks(path) {
		var check = confirm("Wirklich löschen?");
		if (check) {
			const response = await axios.post(uri_wirbelwind_box + "/playlist",
				{
					uuid: this.uuid,
					delete: [path]
				});
			this.tracks = response.data[0].tracks;
		}
	},
	reset() {

	},
	delete() {

	}

})

const manage_playlists = createApp({
	data() {
		return {
			reactive_playlists,
			reactive_selected_playlist,
			reactive_active_playlist
		}
	},
	created() {
		reactive_playlists.update();
	},
	mounted() {
		var source = new EventSource(uri_wirbelwind_box + "/events");
		source.addEventListener('evt_sd_player', function (e) {
			switch (e.data) {
				case "play_track":
					if (reactive_active_playlist.uuid == reactive_selected_playlist.uuid) {
						console.log(reactive_active_playlist.uuid +
							" " + reactive_active_playlist.curr_timestamp +
							" " + reactive_active_playlist.curr_max_timestamp +
							" " + reactive_active_playlist.curr_track);
						reactive_selected_playlist.curr_track = reactive_active_playlist.curr_track;
						reactive_selected_playlist.curr_timestamp = reactive_active_playlist.curr_timestamp;
						reactive_selected_playlist.curr_max_timestamp = reactive_active_playlist.curr_max_timestamp;
					}
					break;
				case "pause_track":
					break;
				case "resume_track":
					break;
				case "stop_track":
					break;
				case "next_track":
					break;
				case "set_playlist":
					break;
				default:
					console.log("Error event not known!");
					break;
			}
			console.log(e.data);
		}, false);
		source.addEventListener('active_curr_playlist', function (e) {
			reactive_active_playlist.uuid = e.data;
			//console.log(reactive_active_playlist.uuid);
		}, false);
		source.addEventListener('active_curr_track', function (e) {
			reactive_active_playlist.curr_track = e.data;
			//console.log(reactive_active_playlist.curr_track);
		}, false);
		source.addEventListener('active_curr_max_timestamp', function (e) {
			reactive_active_playlist.curr_max_timestamp = e.data;
			//console.log(reactive_active_playlist.curr_max_timestamp);
		}, false);
		source.addEventListener('active_curr_timestamp', function (e) {
			reactive_active_playlist.curr_timestamp = e.data;
			//console.log(reactive_active_playlist.curr_timestamp);
		}, false);
	},
	computed: {
		update_uuid: {
			get() {
				//console.log("getter");
				return reactive_selected_playlist.uuid;
			},
			set(uuid) {
				//console.log("setter");
				reactive_selected_playlist.uuid = uuid;
				reactive_selected_playlist.refresh();
			}
		},
		update_name: {
			get() {
				return reactive_selected_playlist.name;

			},
			set(name) {
				reactive_playlists.update();
				reactive_selected_playlist.updateName(name);
			}
		}
	},
	methods: {
		onChangeCurrTimestamp() {
			reactive_selected_playlist.updateCurrTimestamp();
		},
		onChangeVolume() {
			reactive_selected_playlist.updateVolume();
		},
		onChangeTrack(index) {
			reactive_selected_playlist.updateCurrTrack(index);
		}
	}
})
manage_playlists.mount('#app-playlists')

const files = createApp({
	data() {
		return {
			treeData: [],
			reactive_selected_playlist,
			files: ''
		}
	},
	created() {
		fetch(uri_wirbelwind_box + "/files?path=/")
			.then(response => response.json())
			.then(data => (this.treeData = data));
	},
	methods: {
	}
})

files.component("tree-item", {
	template: `
	<li v-if="!this.item.deleted">
	<div>
	{{ item.path.split('/').pop() }}
	<img src="icons/material-icons/folder_open_black_24dp.svg" v-if="isFolder && isOpen" 
	:class="{folder: isFolder}" class="add_curser_pointer"
	v-on:click="toggle">
	<img src="icons/material-icons/folder_black_24dp.svg" v-if="isFolder && !isOpen" 
	:class="{folder: isFolder}" class="add_curser_pointer"
	v-on:click="toggle">
	<img src="icons/material-icons/settings_black_24dp.svg" alt="Einstellungen" v-if="isFolder" class="add_curser_pointer" v-on:click="toggleFolderDialog()">
	<div class="container" v-show="isOpenFolderDialog">
	<form>
	<input v-model.lazy="new_folder_name" type="text" placeholder='' @keypress.enter.prevent />
	</form>
	<img src="icons/material-icons/delete_black_24dp.svg" alt="Ordner löschen" v-if="isFolder" class="add_curser_pointer" v-on:click="deleteFolder()">
	<img src="icons/material-icons/create_new_folder_black_24dp.svg" v-if="isFolder" v-on:click="createFolder(new_folder_name)" class="addfolder add_curser_pointer" :class="{folder: isFolder}">
	<img src="icons/material-icons/upload_file_black_24dp.svg" v-if="isFolder" v-on:click="toggleFileUploadDialog()" class="uploadfile add_curser_pointer" :class="{folder: isFolder}">
	<div class="container" v-show="isOpenFileUploadDialog">
	<div class="large-12 medium-12 small-12 cell">
	<label>Files
	<input name='uploadfiles' type="file" id="files" ref="files" multiple v-on:change="handleFilesUpload()" />
	</label>
	<button v-on:click="submitFiles(path)">Submit</button>
	</div>
	</div>
	</div>
	<img src="icons/material-icons/playlist_add_black_24dp.svg" v-if="!isFolder" v-on:click="addTrackToPlaylist()" class="addtrack add_curser_pointer">
	<img src="icons/material-icons/remove_circle_black_24dp.svg" v-if="!isFolder" v-on:click="deleteFile()" class="deletefile add_curser_pointer">
	</div>
	<ul v-show="isOpen" v-if="isFolder">
	<tree-item
	class="item"
	v-for="(child, index) in item.children"
	v-bind:key="index"
	v-bind:item="child"
	></tree-item>
	</ul>
	</li>`,
	props: {
		item: Object
	},
	data() {
		return {
			isOpen: false,
			isOpenFileUploadDialog: false,
			isOpenFolderDialog: false
		};
	},
	computed: {
		isFolder() {
			return (this.item.children && this.item.children.length) || this.item.type === 'd';
		}
	},
	methods: {
		toggle() {
			if (this.isFolder) {
				this.isOpen = !this.isOpen;
				if (this.isOpen) {
					this.loadChildren();
				}
			}
		},
		loadChildren() {
			fetch(uri_wirbelwind_box + "/files?path=" + this.item.path)
				.then(response => response.json())
				.then(data => this.item.children = data);
		},
		addTrackToPlaylist() {
			console.log(this.item.path)
			if (reactive_selected_playlist.uuid != undefined) {
				reactive_selected_playlist.addTracks(this.item.path);
			} else {
				alert("Select playlist first");
			}
		},
		async deleteFile() {
			var check = confirm("Wirklich löschen?");
			if (check) {
				path = this.item.path.split("/"); path.pop();
				response = await axios.patch(uri_wirbelwind_box + "/files?path=" + path.join("/"), { delete_file: this.item.path });
				this.item.deleted = true;
			}
		},
		toggleFolderDialog() {
			this.isOpenFolderDialog = !this.isOpenFolderDialog;
		},
		async createFolder(name) {
			response = await axios.post(uri_wirbelwind_box + "/files?path=" + this.item.path, { create_folder: this.item.path + "/" + name });
		},
		async deleteFolder() {
			var check = confirm("Wirklich löschen?");
			if (check) {
				path = this.item.path.split("/"); path.pop();
				response = await axios.patch(uri_wirbelwind_box + "/files?path=" + path.join("/"), { delete_folder: this.item.path });
				this.item.deleted = true;
			}
		},
		toggleFileUploadDialog() {
			this.isOpenFileUploadDialog = !this.isOpenFileUploadDialog;
		},
		/*
		Submits all of the files to the server
	  */
		submitFiles() {
			/*
			  Initialize the form data
			*/
			let formData = new FormData();

			/*
			  Iteate over any file sent over appending the files
			  to the form data.
			*/
			for (var i = 0; i < this.files.length; i++) {
				let file = this.files[i];

				formData.append('files[' + i + ']', file);
			}

			// Add Path
			//formData.append("path", "test");

			/*
			  Make the request to the POST /multiple-files URL
			*/
			success = false;
			axios.post(uri_wirbelwind_box + this.item.path,
				formData,
				{
					headers: {
						'Content-Type': 'multipart/form-data'
					}
				}
			).then(function () {
				console.log('SUCCESS!!');
			})
				.catch(function () {
					console.log('FAILURE!!');
				});

			this.loadChildren();
		},
		/*
		  Handles a change on the file upload
		*/
		handleFilesUpload() {
			this.files = this.$refs.files.files;
		}
	}
})
files.mount('#app-files')
