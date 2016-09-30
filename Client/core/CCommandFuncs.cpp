/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CCommandFuncs.cpp
*  PURPOSE:     Implementation of all built-in commands
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Christian Myhre Lundheim <>
*               Derek Abdine <>
*               Ed Lyons <eai@opencoding.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include <game/CGame.h>

extern CCore* g_pCore;
extern bool g_bBoundsChecker;

constexpr size_t MTA_HELP_SPACING = 32;

void CCommandFuncs::Help ( const char* szParameters )
{
    CConsoleInterface* pConsole = g_pCore->GetConsole ();

    if ( szParameters && atoi(szParameters) == 1 )
        CCore::GetSingleton().GetMultiplayer()->AllowWindowsCursorShowing(true);
    else
        CCore::GetSingleton().GetMultiplayer()->AllowWindowsCursorShowing(false);

    pConsole->Print ( _("***[ COMMAND HELP ]***\n") );

    char spacer[MTA_HELP_SPACING+1];
    for ( auto& pCommand : CCommands::GetSingletonPtr()->GetCommands()) {
        const char *szCommandName = pCommand.szCommandName;
        const char *szDescription = pCommand.szDescription;

        // Space out the name and description evenly (tab-like)
        auto spacing = MTA_HELP_SPACING - strlen ( szCommandName );
        if ( spacing <= 0 ) spacing = 1;

        memset(&spacer[0], ' ', MTA_HELP_SPACING);
        spacer[spacing] = NULL;

        pConsole->Printf ( "* %s%s%s\n", pCommand.szCommandName, &spacer[0], pCommand.szDescription );
    }

    pConsole->Printf ( "***[--------------]***\n" );
}

void CCommandFuncs::Exit ( const char* szParameters )
{
    g_pCore->Quit ();
}

void CCommandFuncs::Ver ( const char* szParameters )
{
    ShowCursor(TRUE);
    HCURSOR hc = LoadCursor ( NULL, IDC_ARROW );
    SetCursor ( hc );

    // Compose version string
    unsigned short usNetRev = CCore::GetSingleton ().GetNetwork ()->GetNetRev ();
    unsigned short usNetRel = CCore::GetSingleton ().GetNetwork ()->GetNetRel ();
    SString strVersion = BLUE_VERSION_STRING;
    if ( usNetRev > 0 || usNetRel > 0 )
        strVersion += SString ( ".%d", usNetRev );
    if ( usNetRel > 0 )
        strVersion += SString ( ".%03d", usNetRel );
    strVersion += "\n";
    strVersion += _(BLUE_COPYRIGHT_STRING);
    CLocalGUI::GetSingleton ( ).EchoConsole ( strVersion );
}

void CCommandFuncs::ScreenShot ( const char* szParameters )
{
    g_pCore->TakeScreenShot ();
}

void CCommandFuncs::Time ( const char* szParameters )
{
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    SString strTimeAndDate ( _("* The time is %d:%02d:%02d"), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec );
    CCore::GetSingleton ().ChatEchoColor ( strTimeAndDate, 255, 100, 100 );
}

// this fails randomly, see comments in CConsole
void CCommandFuncs::Clear ( const char* szParameters )
{
    CCore::GetSingleton ().GetConsole ()->Clear ();
}


void CCommandFuncs::Connect ( const char* szParameters )
{
    CModManager::GetSingleton ().Unload ();

    // Any mod loaded?
    if ( !CModManager::GetSingleton ().GetCurrentMod () )
    {
        // Parse the arguments (host port nick pass)
        char szBuffer [256] = "";
        if ( szParameters )
            STRNCPY( szBuffer, szParameters, NUMELMS( szBuffer ) );

        if ( !strncmp( szBuffer, "mtasa://", 8 ) )
        {
            // Using a mtasa:// URI to connect
            SString strArguments = g_pCore->GetConnectCommandFromURI ( szBuffer );

            if ( strArguments.length () > 0 && g_pCore->GetCommands()->Execute ( strArguments ) )
            {
                return;
            }
        }

        char* szHost = strtok ( szBuffer, " " );
        char* szPort = strtok ( NULL, " " );
        char* szNick = strtok ( NULL, " " );
        char* szPass = strtok ( NULL, " " );

        std::string strNick;
        if ( !szNick )  CVARS_GET ( "nick", strNick );
        else            strNick = szNick;
        
        // Got all required arguments?
        if ( !szHost || !szPort || strNick.empty () )
        {
            CCore::GetSingleton ().GetConsole ()->Print ( _("connect: Syntax is 'connect <host> <port> [<nick> <pass>]'") );
            return;
        }

        // Verify and convert the port number
        int iPort = atoi ( szPort );
        if ( iPort <= 0 || iPort > 0xFFFF )
        {
            CCore::GetSingleton ().GetConsole ()->Print ( _("connect: Bad port number") );
            return;
        }

        unsigned short usPort = static_cast < unsigned short > ( iPort );

        // Got a password?
        char emptyPass = 0;
        if ( !szPass )
        {
            szPass = &emptyPass;
        }

        // Start the connect
        if ( CCore::GetSingleton ().GetConnectManager ()->Connect ( szHost, usPort, strNick.c_str (), szPass ) )
        {
            CCore::GetSingleton ().GetConsole ()->Printf ( _("connect: Connecting to %s:%u..."), szHost, usPort );
        }
        else
        {
            CCore::GetSingleton ().GetConsole ()->Printf ( _("connect: could not connect to %s:%u!"), szHost, usPort );
        }
    }
    else
    {
        CCore::GetSingleton ().GetConsole ()->Print ( _("connect: Failed to unload current mod") );
    }
}

void CCommandFuncs::Reconnect ( const char* szParameters )
{
    CModManager::GetSingleton ().Unload ();

    std::string strHost, strNick, strPassword;
    unsigned int uiPort;

    CVARS_GET ( "host",         strHost );
    CVARS_GET ( "nick",         strNick );
    CVARS_GET ( "password",     strPassword );
    CVARS_GET ( "port",         uiPort );

    // Restart the connection.
    CModManager::GetSingleton ().Unload ();

    // Any mod loaded?
    if ( !CModManager::GetSingleton ().GetCurrentMod () )
    {
        
        // Verify and convert the port number
        if ( uiPort <= 0 || uiPort > 0xFFFF )
        {
            CCore::GetSingleton ().GetConsole ()->Print ( _("connect: Bad port number") );
            return;
        }

        unsigned short usPort = static_cast < unsigned short > ( uiPort );

        // Got a password?
        if ( !strPassword.c_str() )
        {
            strPassword = '\0';
        }

        // Start the connect
        if ( CCore::GetSingleton ().GetConnectManager ()->Reconnect ( strHost.c_str(), usPort, strPassword.c_str(), false ) )
        {
            CCore::GetSingleton ().GetConsole ()->Printf ( _("connect: Connecting to %s:%u..."), strHost.c_str(), usPort );
        }
        else
        {
            CCore::GetSingleton ().GetConsole ()->Printf ( _("connect: could not connect to %s:%u!"), strHost.c_str(), usPort );
        }
    }
    else
    {
        CCore::GetSingleton ().GetConsole ()->Print ( "connect: Failed to unload current mod" );
    }
}

void CCommandFuncs::Bind ( const char* szParameters )
{
    CCore::GetSingleton ().GetKeyBinds ()->BindCommand ( szParameters );
    CCore::GetSingleton ().SaveConfig();
}

void CCommandFuncs::Unbind ( const char* szParameters )
{
    CCore::GetSingleton ().GetKeyBinds ()->UnbindCommand ( szParameters );
    CCore::GetSingleton ().SaveConfig();
}

void CCommandFuncs::Binds ( const char* szParameters )
{
    CCore::GetSingleton ().GetKeyBinds ()->PrintBindsCommand ( szParameters );
}

void CCommandFuncs::CopyGTAControls ( const char* szParameters )
{
    CKeyBindsInterface* pKeyBinds = CCore::GetSingleton ().GetKeyBinds ();

    if ( pKeyBinds )
    {
        pKeyBinds->RemoveAllGTAControls ();
        pKeyBinds->LoadControlsFromGTA ();
        CCore::GetSingleton ().GetConsole ()->Print ( _("Bound all controls from GTA") );
    }
}

void CCommandFuncs::HUD ( const char* szParameters )
{
    int iCmd = ( szParameters && szParameters [ 0 ] ) ? atoi ( szParameters ) : -1;
    bool bShow = ( iCmd == 1 ) ? true : ( iCmd == 0 ) ? false : g_pCore->GetGame ()->GetHud ()->IsDisabled ();
    g_pCore->GetGame ()->GetHud ()->Disable ( !bShow );
}

void CCommandFuncs::SaveConfig ( const char* szParameters )
{
    CCore::GetSingleton ().SaveConfig ();
    g_pCore->GetConsole ()->Print ( _("Saved configuration file") );
}

void CCommandFuncs::ChatScrollUp ( const char* szParameters )
{
    CChat* pChat = CCore::GetSingleton ().GetLocalGUI ()->GetChat ();
    if ( pChat ) pChat->Scroll ( atoi ( szParameters ) );
}

void CCommandFuncs::ChatScrollDown ( const char* szParameters )
{
    CChat* pChat = CCore::GetSingleton ().GetLocalGUI ()->GetChat ();
    if ( pChat ) pChat->Scroll ( atoi ( szParameters ) );
}

void CCommandFuncs::DebugScrollUp ( const char* szParameters )
{
    CDebugView* pDebug = CCore::GetSingleton ().GetLocalGUI ()->GetDebugView ();
    if ( pDebug ) pDebug->Scroll ( atoi ( szParameters ) );
}

void CCommandFuncs::DebugScrollDown ( const char* szParameters )
{
    CDebugView* pDebug = CCore::GetSingleton ().GetLocalGUI ()->GetDebugView ();
    if ( pDebug ) pDebug->Scroll ( atoi ( szParameters ) );
}

void CCommandFuncs::DebugClear ( const char* szParameters )
{
    CCore::GetSingleton ().GetLocalGUI ()->GetDebugView ()->Clear ();
}

void CCommandFuncs::Test ( const char* szParameters )
{
    if ( SStringX ( szParameters ) == "ca" )
    {
        SString strStats = CCrashDumpWriter::GetCrashAvertedStatsSoFar ();
        SString strMsg = SString ( "Crash averted stats:\n%s", strStats.empty () ? "None" : *strStats );
        CCore::GetSingleton ().GetConsole ()->Print ( strMsg );
    }
    else
    if ( SStringX ( szParameters ) == "crashme" )
    {
        std::string strNick;
        CVARS_GET ( "nick", strNick );
        if ( strNick == szParameters )
        {
            if ( FileExists ( CalcMTASAPath ( "debug.txt" ) ) )
            {
                int* pData = NULL;
                *pData = 0;
            }
        }
    }
}

void CCommandFuncs::Serial ( const char* szParameters )
{
    // Get our serial
    char szSerial [ 64 ];
    g_pCore->GetNetwork ()->GetSerial (szSerial, sizeof ( szSerial ));

    // Print it
    CCore::GetSingleton ().GetConsole ()->Printf ( _("* Your serial is: %s"), szSerial );
}


void CCommandFuncs::FakeLag ( const char *szCmdLine )
{
#if defined(MTA_DEBUG) || defined(MTA_BETA)

    std::vector < SString > parts;
    SStringX ( szCmdLine ).Split ( " ", parts );

    if ( parts.size () < 3 )
    {
        g_pCore->GetConsole ()->Print ( "fakelag <packet loss> <extra ping> <ping variance> [ <KBPS limit> ]" );
        return;
    }

    int iPacketLoss = atoi ( parts[0] );
    int iExtraPing = atoi ( parts[1] );
    int iExtraPingVary = atoi ( parts[2] );
    int iKBPSLimit = 0;
    if ( parts.size () > 3 )
        iKBPSLimit = atoi ( parts[3] );

    g_pCore->GetNetwork ()->SetFakeLag ( iPacketLoss, iExtraPing, iExtraPingVary, iKBPSLimit );
    g_pCore->GetConsole ()->Print ( SString ( "Client send lag is now: %d%% packet loss and %d extra ping with %d extra ping variance and %d KBPS limit", iPacketLoss, iExtraPing, iExtraPingVary, iKBPSLimit ) );

#endif
}

void CCommandFuncs::ShowMemStat ( const char* szParameters )
{
    int iCmd = ( szParameters && szParameters [ 0 ] ) ? atoi ( szParameters ) : -1;
    bool bShow = ( iCmd == 1 ) ? true : ( iCmd == 0 ) ? false : !GetMemStats ()->IsEnabled ();
    GetMemStats ()->SetEnabled ( bShow );
}

void CCommandFuncs::ShowFrameGraph ( const char* szParameters )
{
    int iCmd = ( szParameters && szParameters [ 0 ] ) ? atoi ( szParameters ) : -1;
    bool bShow = ( iCmd == 1 ) ? true : ( iCmd == 0 ) ? false : !GetGraphStats ()->IsEnabled ();
    GetGraphStats ()->SetEnabled ( bShow );
}

void CCommandFuncs::JingleBells ( const char* szParameters )
{
    g_strJingleBells = szParameters;
    g_pCore->GetConsole ()->Print ( "Batman smells" );
}