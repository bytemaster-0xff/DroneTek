using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.System.Threading;

namespace NiVek.GroundStation.Inputs
{
    public class FlightController
    {
        private GamePad _gamePad;
        private Drone _drone;
        ThreadPoolTimer _throttleUpTimer;
        ThreadPoolTimer _throttleDownTimer;

        public Drone Drone
        {
            get { return _drone; }
            set
            {
                _drone = value;
            }
        }

        double lastT;

        public void StartListeningFlightStick()
        {
            _gamePad = new GamePad();
            _gamePad.ButtonStateChanged += _gamePad_ButtonStateChanged;

            var stickTimer = ThreadPoolTimer.CreatePeriodicTimer((obj) =>
            {
                if (_gamePad.HasState)
                {
                    var x = (sbyte)((_gamePad.GetState().RightX / 128.0) * 30);
                    var y = (sbyte)((_gamePad.GetState().RightY / 128.0) * 30);

                    if (x != Drone.RollInput || y != Drone.PitchInput)
                    {
                        Drone.RollInput = (sbyte)x;
                        Drone.PitchInput = (sbyte)-y;
                        Drone.SendControlUpdate();
                        //    Debug.WriteLine("Roll -> " + x + "   Pitch -> " + y);
                    }

                    var z = (sbyte)((_gamePad.GetState().LeftX / 128.0) * 90);
                    var t = (sbyte)((_gamePad.GetState().LeftY / 128.0) * 90);

                    if (lastT != t)
                    {
                        //  Debug.WriteLine("Throttle -> " + t + "  Yaw -> " + z);
                        lastT = t;
                    }
                    if (Math.Abs(z) < 10)
                        z = 0;

                    if (z != Drone.YawInput)
                    {
                        Drone.YawInput = (sbyte)z;
                        Drone.SendControlUpdate();
                    }



                }
            }, TimeSpan.FromMilliseconds(50));
        }

        void _gamePad_ButtonStateChanged(object sender, GamePad.ButtonStateChangedEventArgs e)
        {
            HandleButtonChange(e.Button, e.ButtonState);
        }

        public void HandleButtonChange(GamePad.GamePadButtons button, GamePad.ButtonState state)
        {
            switch (button)
            {
                case GamePad.GamePadButtons.Start:
                    if (state == GamePad.ButtonState.Pressed)
                    {
                        Drone.Arm();
                        Drone.StartRecording();
                    }

                    break;
                case GamePad.GamePadButtons.Back:
                    if (state == GamePad.ButtonState.Pressed)
                    {
                        Drone.StopRecording();
                        Drone.Safe();
                    }
                    break;

                case GamePad.GamePadButtons.A:
                    if (state == GamePad.ButtonState.Pressed)
                        Drone.Land();
                    break;

                case GamePad.GamePadButtons.Y:
                    if (state == GamePad.ButtonState.Pressed)
                    {
                        Debug.WriteLine("Launched was pressed");
                        Drone.Launch();
                    }

                    break;

                case GamePad.GamePadButtons.X:
                    if (state == GamePad.ButtonState.Pressed)
                    {
                        Drone.StopRecording();
                        Drone.Kill();
                    }
                    break;

                case GamePad.GamePadButtons.B:
                    if (state == GamePad.ButtonState.Pressed)
                        Drone.ToggleAltitudeHold();
                    break;


                case GamePad.GamePadButtons.LeftTrigger:
                    if (state == GamePad.ButtonState.Pressed)
                    {
                        _throttleDownTimer = ThreadPoolTimer.CreatePeriodicTimer((obj) =>
                        {
                            Drone.ThrottleDown();
                        }, TimeSpan.FromMilliseconds(Drone.Status.AltitudeHold == SystemStatus.AltitudeHoldEnum.On ? 250 : 50));
                    }
                    else
                    {
                        _throttleDownTimer.Cancel();
                        _throttleDownTimer = null;
                    }
                    break;

                case GamePad.GamePadButtons.RightTrigger:
                    if (state == GamePad.ButtonState.Pressed)
                    {
                        _throttleUpTimer = ThreadPoolTimer.CreatePeriodicTimer((obj) =>
                        {
                            Drone.ThrottleUp();
                        }, TimeSpan.FromMilliseconds(Drone.Status.AltitudeHold == SystemStatus.AltitudeHoldEnum.On ? 250 : 50));
                    }
                    else
                    {
                        _throttleUpTimer.Cancel();
                        _throttleUpTimer = null;
                    }
                    break;

            }
        }

    }
}
