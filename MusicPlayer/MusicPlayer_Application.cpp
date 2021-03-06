#include "MusicPlayer_Application.h"


MusicPlayer::MusicPlayer(sf::RenderWindow& _window) :
	window_(_window)
{
	window_.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.

	// Desired Window Settings, removed for more user customization, use NoMove, NoCollapse and NoResize for more "User Friendly" and clean approach
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	//window_flags |= ImGuiWindowFlags_NoTitleBar;
	//window_flags |= ImGuiWindowFlags_NoResize;	//commented out because for some reason if resize is disabled defaults to minimum sizes rather than predefined preferences.

	//if (!LoadMusic(FileList))
	//	throw std::runtime_error("Invalid File Name Detected, Exiting");	//Doesnt work - sends exception but doesnt show message. - If uncommenting this make sure to uncomment the try/catch in main.cpp

	// Initilize Values
	isVolume = true;
	isTrackList = true;
	TrackArtID = 99;	//set to any number other than 0 so it resets when checking in Update();

	MusicTrack.Import(MusicDirectory);
	SongList = MusicTrack.List();
	for (unsigned int i = 0; i < MusicTrack.List().size(); i++)
	{
		std::cout << "App Storing Texture: " << MusicTrack.ArtPath(i) << std::endl;
		sf::Texture tempTexture;
		tempTexture.loadFromFile(MusicTrack.ArtPath(i));
		TrackArtTexture.push_back(tempTexture);	//no need to check this since its already checked in Track Class
	}

	if (MusicTrack.List().size() == 0)
	{
		std::cout << "\n\n NO MUSIC FOUND !!!\nPlace .ogg files /music/ folder." << std::endl;
	}
}

MusicPlayer::~MusicPlayer()
{ }

bool MusicPlayer::Run()
{
	if (MusicTrack.List().size() != 0)	//Only Update if music has been loaded/stored
		Update();

	return 0;
}

void MusicPlayer::CleanUp()
{
	// Clean Up
	ImGui::SFML::Shutdown();
}

bool MusicPlayer::Update()
{
	static float volume = 50.f;

	while (window_.isOpen())	// Game Loop
	{
		MusicTrack.Update();	//Not required for music to play, but required for checks such as checking if track is finished and to go to next track

		sf::Event event;
		while (window_.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			if (event.type == sf::Event::Closed)
				window_.close();
		}

		ImGui::SFML::Update(window_, deltaClock.restart());
		//ImGui::ShowTestWindow();	//ImGui Demo/Example Window

		ImGui::Begin("Player", nullptr, window_flags); // begin window

		ImGui::Text(MusicTrack.Name().c_str());
		ImGui::Text(MusicTrack.Artist().c_str());

		std::string SecondsString = std::to_string(int(MusicTrack.CurrentTime()) % 60);
		if (SecondsString.size() == 1)
			SecondsString = "0" + SecondsString;
		std::string MinutesString = std::to_string(int(MusicTrack.CurrentTime() / 60.0f));
		if (MinutesString.size() == 1)
			MinutesString = "0" + MinutesString;

		std::string TrackTimeString = MinutesString + ":" + SecondsString;

		const char* TimeChar = TrackTimeString.c_str();
		ImGui::Text(TimeChar); // FIXME this seems hacky? float>string>const char*
		ImGui::SameLine();
		if (isSeeker)
		{
			static float CurrentTrackTime = MusicTrack.CurrentTime();
			CurrentTrackTime = MusicTrack.CurrentTime();
			ImGui::SliderFloat("", &CurrentTrackTime, 0.0f, MusicTrack.Duration(), "");
			if (CurrentTrackTime != MusicTrack.CurrentTime())
				MusicTrack.SetCurrentTime(CurrentTrackTime);
		}
		else
		{
			ImGui::ProgressBar(MusicTrack.CurrentTime() / MusicTrack.Duration(), ImVec2(0.0f, 0.0f));
		}
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

		// Reuse previous Temp Strings to create new
		SecondsString = std::to_string(int(MusicTrack.Duration()) % 60);
		if (SecondsString.size() == 1)
			SecondsString = "0" + SecondsString;
		MinutesString = std::to_string(int(MusicTrack.Duration() / 60.0f));
		if (MinutesString.size() == 1)
			MinutesString = "0" + MinutesString;

		std::string TrackDurationString = MinutesString + ":" + SecondsString;
		TimeChar = TrackDurationString.c_str();
		ImGui::Text(TimeChar);

		if (ImGui::Button("|<"))	//Previous Song
		{
			MusicTrack.Prev();
		} ImGui::SameLine();

		if (ImGui::Button("STOP"))	//Stop Playback
		{
			MusicTrack.Stop();
		} ImGui::SameLine();

		if (ImGui::Button((MusicTrack.isPlaying()) ? (cPause) : (cPlay)))	// Play/Pause Playback
		{
			MusicTrack.PlayPause();
		} ImGui::SameLine();

		if (ImGui::Button("Loop"))	//Loop Current Song
		{
			MusicTrack.Loop();
		} ImGui::SameLine();

		if (ImGui::Button(">|"))	//Previous Song
		{
			MusicTrack.Next();
		}

		if (ImGui::Button("Volume"))	//Toggle Show Volume
		{
			isVolume = !isVolume;
		} ImGui::SameLine();

		if (ImGui::Button("Seeker"))	//Toggle Seeker Slider
		{
			isSeeker = !isSeeker;
		} ImGui::SameLine();

		if (ImGui::Button("Track List"))	//Toggle Track List Window
		{
			isTrackList = !isTrackList;
		}

		ImGui::End(); // End player window

		if (isVolume)
		{
			ImGui::Begin("Volume", nullptr, window_flags);	// Begin Track List Window
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor::HSV(7.0f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImColor::HSV(7.0f, 0.6f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImColor::HSV(7.0f, 0.7f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImColor::HSV(7.0f, 0.9f, 0.9f));
			ImGui::VSliderFloat("##v", ImVec2(18, 160), &volume, 0.0f, 100.0f, "");

			if (ImGui::IsItemActive() || ImGui::IsItemHovered())
				ImGui::SetTooltip("%.3f", volume);
			ImGui::PopStyleColor(4);

			if (ImGui::IsItemActive())	//Is Slider being used
				MusicTrack.Volume(volume);

			ImGui::End();	// End Track List Window
		}

		if (isTrackList)
		{
			ImGui::Begin("Track List", nullptr, window_flags);	// Begin Track List Window

			/*
			Custom ListBox due to ImGui ListBox not supporting unlimited height.
			This ListBox will attack to the bottom of the window.
			Also makes use of vector instead of array since size of list may be unknown.
			*/
			if (ImGui::ListBoxHeader("", ImVec2(-10.0f, -10.0f)))
			{
				// Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
				bool value_changed = false;
				ImGuiListClipper clipper(MusicTrack.List().size(), ImGui::GetTextLineHeightWithSpacing());
				while (clipper.Step())
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						const bool item_selected = (i == MusicTrack.CurrentTrackID());
						const char* item_text = SongList[i].c_str();

						ImGui::PushID(i);
						if (ImGui::Selectable(item_text, item_selected))
						{
							//current_track = i;
							MusicTrack.SetTrackID(i);
							value_changed = true;
						}
						ImGui::PopID();
					}
				ImGui::ListBoxFooter();
			}	// End Custom ListBox
			ImGui::End();	// End Track List Window
		}

		// Check if Art needs updated
		if (TrackArtID != MusicTrack.CurrentTrackID())
		{
			TrackArt.setTexture(TrackArtTexture[MusicTrack.CurrentTrackID()]);
			TrackArtID = MusicTrack.CurrentTrackID();
		}

		window_.clear();
		window_.draw(TrackArt);
		ImGui::SFML::Render(window_);
		window_.display();
	} // End of window_isOpen()

	return 0;
}