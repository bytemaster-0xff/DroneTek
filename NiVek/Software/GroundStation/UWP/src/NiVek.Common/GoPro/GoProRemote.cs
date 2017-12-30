using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace NiVek.Common.GoPro
{
    public class GoProRemote
    {
        public class CameraSetting
        {
            public String Name { get; set; }
            public String Url { get; set; }
        }


        public const string CAMERA_ON = "/PW?t={0}&p=%01";
        public const string CAMERA_OFF = "/PW?t={0}&p=%00";
        public const string CHANGE_MODE = "/PW?t={0}&p=%02";

        public List<CameraSetting> CameraPower = new List<CameraSetting>() 
        {
            new CameraSetting() {Name="On", Url = CAMERA_ON},
            new CameraSetting() {Name="On", Url = CAMERA_OFF},
        };

        public const string START_CAPTURE = "/SH?t={0}&p=%01";
        public const string END_CAPTURE = "/SH?t={0}&p=%00";

        public const string PREVIEW_ON = "/PV?t={0}&p=%02";
        public const string PREVIEW_OFF = "/PV?t={0}&p=%00";

        public const string MODE_CAMERA = "/CM?t={0}&p=%00";
        public const string MODE_PHOTO = "/CM?t={0}&p=%01";
        public const string MODE_BURST = "/CM?t={0}&p=%02";
        public const string MODE_TIMELAPSE1 = "/CM?t={0}&p=%03";
        public const string MODE_TIMELAPSE2 = "/CM?t={0}&p=%04";


        public List<CameraSetting> CameraMode = new List<CameraSetting>() 
        {
            new CameraSetting() {Name="Camera", Url = MODE_CAMERA},
            new CameraSetting() {Name="Photo", Url = MODE_PHOTO},
            new CameraSetting() {Name="Burst", Url = MODE_BURST},
            new CameraSetting() {Name="Time Lapse", Url = MODE_TIMELAPSE1},
        };

        public const string ORIENTATION_HEAD_UP = "/UP?t={0}&p=%00";
        public const string ORIENTATION_HEAD_DOWN = "/UP?t={0}&p=%01";


        public const string VIDEO_WVGA_60_HZ = "/VR?t={0}&p=%00";
        public const string VIDEO_WVGA_120_HZ = "/VR?t={0}&p=%01";
        public const string VIDEO_720_30_HZ = "/VR?t={0}&p=%02";
        public const string VIDEO_720_60_HZ = "/VR?t={0}&p=%03";
        public const string VIDEO_960_30_HZ = "/VR?t={0}&p=%04";
        public const string VIDEO_960_60_HZ = "/VR?t={0}&p=%05";
        public const string VIDEO_1080_30_HZ = "/VR?t={0}&p=%06";


        public const string VIDEO_WIDE_FOV_WIDE = "/FV?t={0}&p=%00";
        public const string VIDEO_MEDIUM_FOV_WIDE = "/FV?t={0}&p=%01";
        public const string VIDEO_NARROW_FOV_WIDE = "/FV?t={0}&p=%02";


        public const string VIDEO_11MP_WIDE = "/PR?t={0}&p=%00";
        public const string VIDEO_8MP_MEDIUM = "/PR?t={0}&p=%01";
        public const string VIDEO_5MP_WIDE = "/PR?t={0}&p=%02";
        public const string VIDEO_5MP_MEDIUM = "/PR?t={0}&p=%03";



        public const string TIMER_0_5_SECONDS = "/TI?t={0}&p=%00";
        public const string TIMER_1_SECOND = "/TI?t={0}&p=%01";
        public const string TIMER_2_SECONDS = "/TI?t={0}&p=%02";
        public const string TIMER_5_SECONDS = "/TI?t={0}&p=%03";
        public const string TIMER_10_SECONDS = "/TI?t={0}&p=%04";
        public const string TIMER_30_SECONDS = "/TI?t={0}&p=%05";
        public const string TIMER_60_SECONDS = "/TI?t={0}&p=%06";


        public const string BLIP_VOL_0 = "/BS?t={0}&p=%00";
        public const string BLIP_VOL_70 = "/BS?t={0}&p=%01";
        public const string BLIP_VOL_100 = "/BS?t={0}&p=%02";

        public async void SendCommand(String url)
        {
            var passwordQueryString = String.Format(url, "goprohero");

            url = String.Format("http://10.5.5.9/camera{0}", passwordQueryString);
            Debug.WriteLine(url);
            //10.5.5.109:8080/live/amba.m3u8

            var client = new HttpClient();
            try
            {
                var response = await client.GetAsync(url);
                Debug.WriteLine(await response.Content.ReadAsStringAsync());
            }
            catch(Exception)
            {

            }
        }
    }
}
