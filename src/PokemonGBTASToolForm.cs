using System;
using System.Drawing;
using System.IO;
using System.Reflection;

using BizHawk.Client.Common;
using BizHawk.Client.EmuHawk;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.Dolphin;

namespace PokemonGBTASTool
{
	[ExternalTool("Pokemon GB TAS Tool", Description = "A tool to help with TASing GB Pokemon games.")]
	[ExternalToolApplicability.SingleSystem(VSystemID.Raw.GC)]
	[ExternalToolEmbeddedIcon("PokemonGBTASTool.res.icon.ico")]
	public partial class PokemonGBTASToolForm : ToolFormBase, IExternalToolForm
	{
		public ApiContainer? ApiContainer { get; set; }
		private ApiContainer APIs => ApiContainer!;

		[RequiredService]
		private ISettable<Dolphin.DolphinSettings, Dolphin.DolphinSyncSettings>? Settable { get; set; }

		private StreamWriter? RngSeedWriter { get; set; }
		private StreamWriter RngSeeds => RngSeedWriter!;

		protected override string WindowTitleStatic => "Pokemon GB TAS Tool";

		public PokemonGBTASToolForm()
		{
			InitializeComponent();
			Icon = new Icon(Assembly.GetExecutingAssembly().GetManifestResourceStream("PokemonGBTASTool.res.icon.ico"));
			RngSeedWriter = new(new FileStream($"colo_rng_seeds_{DateTime.Now.Ticks}.txt", FileMode.CreateNew, FileAccess.Write));
			Closing += (sender, args) =>
			{
				RngSeedWriter?.Flush();
				RngSeedWriter?.Close();
				RngSeedWriter?.Dispose();
				RngSeedWriter = null;
			};
		}

		private Dolphin.DolphinSyncSettings? SyncSettings;

		private static readonly DateTime _unixEpoch = new(1970, 1, 1, 0, 0, 0);

        private uint _startingSeed = 0x24113DA9 + 9 + 4;
		private int _seedsChecked = 0;

        public override void UpdateValues(ToolFormUpdateType type)
		{
			if (SyncSettings is null)
            {
				SyncSettings = Settable!.GetSyncSettings();
				SyncSettings.MainSettings.EnableCustomRTC = true;
				SyncSettings.MainSettings.CustomRTCValue = _unixEpoch.AddSeconds(95261391);
				//_startingSeed += 0 * 40500000;
				Settable!.PutSyncSettings(SyncSettings);
				APIs.EmuClient.RebootCore();
			}

			if (_seedsChecked == 32)
            {
				Close();
				return;
			}

			if (APIs.Emulation.FrameCount() == 120)
            {
				APIs.Memory.SetBigEndian();
				var t = SyncSettings!.MainSettings.CustomRTCValue;
				var seed = APIs.Memory.ReadU32(0x80478c90);

				RngSeeds.WriteLine($"" +
                    $"{(uint)(t - _unixEpoch).TotalSeconds}" +
                    $" : 0x{seed:X08}"/* +
					$" : 0x{_startingSeed:X08}"
                    $" : {(int)(seed - _startingSeed)}"*/);

				//_startingSeed += unchecked((uint)(86400 * 40500000UL));
				SyncSettings!.MainSettings.CustomRTCValue = t.AddSeconds(134217728);
				Settable!.PutSyncSettings(SyncSettings!);

				APIs.EmuClient.RebootCore();

				_seedsChecked++;
			}
		}
	}
}
