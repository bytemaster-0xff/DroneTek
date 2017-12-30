using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace NiVek.GroundStation.Controls
{
    public sealed partial class BlackboxFilePicker : UserControl
    {
        public event EventHandler<StorageFile> FileSelected;

        public BlackboxFilePicker()
        {
            this.InitializeComponent();
        }

        public async void Show()
        {
            this.Visibility = Windows.UI.Xaml.Visibility.Visible;

            await RefreshFiles();
        }

        private async Task RefreshFiles()
        {
            var files = await Windows.Storage.ApplicationData.Current.RoamingFolder.GetFilesAsync();
            BBDataFiles.ItemsSource = files.OrderByDescending(fil => fil.Name);
        }

        private void BBDataFiles_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 1)
            {
                if (FileSelected != null)
                    FileSelected(this, e.AddedItems[0] as StorageFile);

                this.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }
        }

        private async void Button_Click_1(object sender, RoutedEventArgs e)
        {
            var file = (sender as Button).DataContext as StorageFile;

            await file.DeleteAsync(StorageDeleteOption.PermanentDelete);

            await RefreshFiles();
        }

        private void Cancel_Click_1(object sender, RoutedEventArgs e)
        {
            this.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
        }
    }
}
