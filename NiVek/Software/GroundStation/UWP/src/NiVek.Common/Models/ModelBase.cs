using NiVek.Common.Comms;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Linq.Expressions;
using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;
using System.Diagnostics;
using Newtonsoft.Json;

namespace NiVek.Common.Models
{
    [AttributeUsage(AttributeTargets.Property)]
    public class DeviceSettingAttribute : Attribute
    {

        public ushort SettingAddress { get; set; }

        private float _scalingFactor = 1.0f;
        
        public float ScalingFactor {
            get { return _scalingFactor; }
            set { _scalingFactor = value; }
        }
    }

    [AttributeUsage(AttributeTargets.Class)]
    public class ModuleConfigAttribute : Attribute
    {
        public byte MessageId { get; set; }
        public NiVek.Common.Comms.Common.ModuleTypes ModuleType { get; set; }
    }



    [DataContract]
    public class ModelBase : System.ComponentModel.INotifyPropertyChanged
    {
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;

        public const byte CMD_SetCfg_Value = 0xC0;

        public const byte MeasureTypeNone = 0;
        public const byte MeasureTypePitch = 10;
        public const byte MeasureTypeRoll = 11;
        public const byte MeasureTypeHeading = 12;
        public const byte MeasureTypeAltitude = 13;

        public enum SensorIds
        {
            MPU6050_Acc = 10,
            MPU6050_Gyro = 11,

            HMC5883_Mag = 20,

            MSU5611_Baro = 30,

            IMU_Attitude = 40,
            IMU_Heading = 41,
            IMU_Altitude = 42,
        }

        public const byte SNSR_ID_CompFilter = 10;

        public class SettingsValue
        {
            public byte Value { get; set; }
            public String Display { get; set; }
        }


        protected bool Set<T>(
            ref T field,
            T newValue = default(T),
            bool broadcast = false,
            [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, newValue))
            {
                return false;
            }

            field = newValue;

            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));

            return true;
        }

        protected void RaisePropertyChanged<TResult>(Expression<Func<TResult>> propertyExpression)
        {
            if (!this.CheckExpressionForMemberAccess(propertyExpression.Body))
                throw new ArgumentException("propertyExpression",
                        string.Format("The expected expression is no 'MemberAccess'; its a '{0}'", propertyExpression.Body.NodeType));


            if (propertyExpression == null)
                throw new ArgumentNullException("propertyExpression");

            if (this.PropertyChanged != null)
                this.PropertyChanged(this, new PropertyChangedEventArgs(this.GetPropertyNameFromExpression(propertyExpression)));
        }


        protected void OnPropertyChanged<TResult>(Expression<Func<TResult>> propertyExpression)
        {
            if (!this.CheckExpressionForMemberAccess(propertyExpression.Body))
                throw new ArgumentException("propertyExpression",
                        string.Format("The expected expression is no 'MemberAccess'; its a '{0}'", propertyExpression.Body.NodeType));


            if (propertyExpression == null)
                throw new ArgumentNullException("propertyExpression");

            if (this.PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(this.GetPropertyNameFromExpression(propertyExpression)));
        }

        private bool CheckExpressionForMemberAccess(System.Linq.Expressions.Expression propertyExpression)
        {
            return propertyExpression.NodeType == ExpressionType.MemberAccess;
        }

        public string GetPropertyNameFromExpression<TResult>(Expression<Func<TResult>> propertyExpression)
        {
            System.Linq.Expressions.MemberExpression memberExpression = (System.Linq.Expressions.MemberExpression)propertyExpression.Body;

            if (memberExpression != null)
            {
                return memberExpression.Member.Name;
            }
            else
                throw new ArgumentException("propertyExpression");
        }

        private Drone _drone;
        [JsonIgnore]
        public Drone Drone
        {
            get { return _drone; }
            set { _drone = value; }
        }

        public async void Send(SensorIds sensorId, byte settingId, short value)
        {
            var msg = new OutgoingMessage()
            {
                DestinationAddress = Drone.DroneAddress,
                ExpectACK = true,
                ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor,
                MessageId = CMD_SetCfg_Value,
            };

            msg.AddByte((byte)sensorId);
            msg.AddByte(settingId);
            msg.Add(value);

            await Drone.SendMessageAsync(msg);
        }

        public async void SendByte(byte sensorId, byte settingId, byte value)
        {
            var msg = new OutgoingMessage()
            {
                DestinationAddress = Drone.DroneAddress,
                ExpectACK = true,
                ModuleType = NiVek.Common.Comms.Common.ModuleTypes.Sensor,
                MessageId = CMD_SetCfg_Value,
            };

            msg.AddByte((byte)sensorId);
            msg.AddByte(settingId);
            msg.AddByte(value);

            await Drone.SendMessageAsync(msg);
        }

        ObservableCollection<SettingsValue> _measureTypes = new ObservableCollection<SettingsValue>()
        {
            new SettingsValue() {Display="None", Value=MeasureTypeNone},
            new SettingsValue() {Display="Pitch", Value=MeasureTypePitch},
            new SettingsValue() {Display="Roll", Value=MeasureTypeRoll},
            new SettingsValue() {Display="Heading", Value=MeasureTypeHeading},
            new SettingsValue() {Display="Altitude", Value=MeasureTypeAltitude},
        };

        [JsonIgnore]
        public ObservableCollection<SettingsValue> MeasureTypes { get { return _measureTypes; } }

        public void Serialize()
        {
            var properties = this.GetType().GetRuntimeProperties();
            var idx = 1;
            foreach (var property in properties.Where(atr => atr.GetCustomAttribute<DeviceSettingAttribute>() != null).OrderBy(atr => atr.GetCustomAttribute<DeviceSettingAttribute>().SettingAddress))
            {
                var attr = property.GetCustomAttribute<DeviceSettingAttribute>();
                Debug.WriteLine("{0}. {1} Attr {2} = {3} * {4}", idx++, attr.SettingAddress, property.Name, property.GetValue(this), attr.ScalingFactor);
            }
        }


        public void Set<TResult>(double newValue, ref byte field, Expression<Func<TResult>> propertyExpression)
        {
            if (field != newValue)
            {
                if (!this.CheckExpressionForMemberAccess(propertyExpression.Body))
                    throw new ArgumentException("propertyExpression",
                            string.Format("The expected expression is no 'MemberAccess'; its a '{0}'", propertyExpression.Body.NodeType));

                if (propertyExpression == null)
                    throw new ArgumentNullException("propertyExpression");

                var memberExpression = ((MemberExpression)propertyExpression.Body).Member;
                var settingAttr = memberExpression.GetCustomAttribute<DeviceSettingAttribute>();

                var moduleInfo = this.GetType().GetTypeInfo().GetCustomAttribute<ModuleConfigAttribute>();

                var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = moduleInfo.MessageId, ModuleType = moduleInfo.ModuleType, ExpectACK = true };
                if(moduleInfo.MessageId == Modules.SensorModule.CMD_SetCfg_Value && moduleInfo.ModuleType == Comms.Common.ModuleTypes.Sensor)
                    msg.AddByte((byte)(settingAttr.SettingAddress >> 8));

                msg.AddByte((byte)settingAttr.SettingAddress);
                msg.Add((short)(newValue));
                UpdateDrone(msg);

                if (this.PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs(this.GetPropertyNameFromExpression(propertyExpression)));
            }  
        }

        public void Set<TResult>(byte newValue, ref byte field, Expression<Func<TResult>> propertyExpression)
        {
            if (field != newValue)
            {
                if (!this.CheckExpressionForMemberAccess(propertyExpression.Body))
                    throw new ArgumentException("propertyExpression",
                            string.Format("The expected expression is no 'MemberAccess'; its a '{0}'", propertyExpression.Body.NodeType));

                if (propertyExpression == null)
                    throw new ArgumentNullException("propertyExpression");

                var memberExpression = ((MemberExpression)propertyExpression.Body).Member;
                var settingAttr = memberExpression.GetCustomAttribute<DeviceSettingAttribute>();

                var moduleInfo = this.GetType().GetTypeInfo().GetCustomAttribute<ModuleConfigAttribute>();

                var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = moduleInfo.MessageId, ModuleType = moduleInfo.ModuleType, ExpectACK = true };
                if (moduleInfo.MessageId == Modules.SensorModule.CMD_SetCfg_Value && moduleInfo.ModuleType == Comms.Common.ModuleTypes.Sensor)
                { 
                    msg.Add(settingAttr.SettingAddress);
                    msg.AddByte(newValue);
                }
                else
                {
                    msg.AddByte((byte)settingAttr.SettingAddress);
                    msg.Add((short)(newValue));
                }

                UpdateDrone(msg);

                if (this.PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs(this.GetPropertyNameFromExpression(propertyExpression)));
            }
        }

        private async void UpdateDrone(OutgoingMessage msg)
        {
            await Drone.SendMessageAsync(msg);
        }


        public void Set<TResult>(double newValue, ref double field, Expression<Func<TResult>> propertyExpression)
        {
            if (field != newValue)
            {
                if (!this.CheckExpressionForMemberAccess(propertyExpression.Body))
                    throw new ArgumentException("propertyExpression",
                            string.Format("The expected expression is no 'MemberAccess'; its a '{0}'", propertyExpression.Body.NodeType));

                if (propertyExpression == null)
                    throw new ArgumentNullException("propertyExpression");

                var memberExpression = ((MemberExpression)propertyExpression.Body).Member;
                var settingAttr = memberExpression.GetCustomAttribute<DeviceSettingAttribute>();

                var moduleInfo = this.GetType().GetTypeInfo().GetCustomAttribute<ModuleConfigAttribute>();

                var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = moduleInfo.MessageId, ModuleType = moduleInfo.ModuleType, ExpectACK = true };
                if (moduleInfo.MessageId == Modules.SensorModule.CMD_SetCfg_Value && moduleInfo.ModuleType == Comms.Common.ModuleTypes.Sensor)
                    msg.AddByte((byte)(settingAttr.SettingAddress >> 8));


                msg.AddByte((byte)settingAttr.SettingAddress);
                msg.Add((short)(newValue * settingAttr.ScalingFactor));
                UpdateDrone(msg);

                if (this.PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs(this.GetPropertyNameFromExpression(propertyExpression)));
            }
        }

        public void Set<TResult>(bool newValue, ref byte field, Expression<Func<TResult>> propertyExpression)
        {
            if (field != (newValue ? 0x255 : 0))
            {
                if (!this.CheckExpressionForMemberAccess(propertyExpression.Body))
                    throw new ArgumentException("propertyExpression",
                            string.Format("The expected expression is no 'MemberAccess'; its a '{0}'", propertyExpression.Body.NodeType));

                if (propertyExpression == null)
                    throw new ArgumentNullException("propertyExpression");

                var memberExpression = ((MemberExpression)propertyExpression.Body).Member;
                var settingAttr = memberExpression.GetCustomAttribute<DeviceSettingAttribute>();

                var moduleInfo = this.GetType().GetTypeInfo().GetCustomAttribute<ModuleConfigAttribute>();

                var msg = new OutgoingMessage() { DestinationAddress = Drone.DroneAddress, MessageId = moduleInfo.MessageId, ModuleType = moduleInfo.ModuleType, ExpectACK = true };
                if (moduleInfo.MessageId == Modules.SensorModule.CMD_SetCfg_Value && moduleInfo.ModuleType == Comms.Common.ModuleTypes.Sensor)
                {
                    msg.Add(settingAttr.SettingAddress);
                    msg.AddByte((byte)(newValue ? 0x255 : 0x00));
                }
                else
                {
                    msg.AddByte((byte)settingAttr.SettingAddress);
                    msg.Add((byte)0x00);
                    msg.AddByte((byte)(newValue ? 0x255 : 0x00));
                }
                
                UpdateDrone(msg);

                if (this.PropertyChanged != null)
                    PropertyChanged(this, new PropertyChangedEventArgs(this.GetPropertyNameFromExpression(propertyExpression)));
            }
        }
    }
}
