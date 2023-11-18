using System.Diagnostics;

namespace simple_vad_gui
{
    public partial class Form1 : Form
    {
        private Dictionary<string, string> param = new Dictionary<string, string>();

        private void updateParam(string key, string value)
        {
            if (param.ContainsKey(key))
            {
                param[key] = value;
            }
            else
            {
                param.Add(key, value);
            }
        }
        public Form1()
        {
            InitializeComponent();
        }

        private void checkBoxUseFiltering_CheckedChanged(object sender, EventArgs e)
        {
        }

        private void inputButton_Click(object sender, EventArgs e)
        {
            // Create an instance of OpenFileDialog
            OpenFileDialog openFileDialog = new OpenFileDialog();

            // Set properties for the OpenFileDialog
            openFileDialog.Title = "选择音频文件";
            openFileDialog.Filter = "Audio Files (*.wav)|*.wav";
            openFileDialog.FilterIndex = 1; // Default to the second filter (Text Files)
            openFileDialog.RestoreDirectory = true;

            // Show the OpenFileDialog and check if the user clicked OK
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                // Get the selected file name
                string selectedFileName = openFileDialog.FileName;

                inputTextBox.Text = selectedFileName;
                executeButton.Enabled = selectedFileName.Length > 0;
            }
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            updateParam("--energy-threshold", energyThreshPicker.Value.ToString());
        }

        private void outFilteredBtn_Click(object sender, EventArgs e)
        {
            // Create an instance of OpenFileDialog
            SaveFileDialog dialog = new SaveFileDialog();

            // Set properties for the OpenFileDialog
            dialog.Title = "选择保存音频文件的位置";
            dialog.Filter = "Audio Files (*.wav)|*.wav";
            dialog.FilterIndex = 1; // Default to the second filter (Text Files)
            dialog.RestoreDirectory = true;

            // Show the OpenFileDialog and check if the user clicked OK
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                // Get the selected file name
                string selectedFileName = dialog.FileName;
                outFilteredTextBox.Text = selectedFileName;

                updateParam("--out-filtered", "\"" + selectedFileName + "\"");
            }


        }

        async Task RunExecutableAsync(string path, string arguments)
        {
            try
            {
                using (Process process = new Process())
                {
                    process.StartInfo.FileName = path;
                    process.StartInfo.Arguments = arguments;
                    process.StartInfo.UseShellExecute = false;
                    process.StartInfo.RedirectStandardOutput = true;
                    process.StartInfo.RedirectStandardError = true;
                    process.StartInfo.CreateNoWindow = true;

                    //* Set your output and error (asynchronous) handlers
                    process.OutputDataReceived += new DataReceivedEventHandler((sender, e) => OutputHandler(sender, e, stdoutBox));
                    process.ErrorDataReceived += new DataReceivedEventHandler((sender, e) => OutputHandler(sender, e, stdoutBox));
                    //* Start process and handlers
                    process.Start();
                    process.BeginOutputReadLine();
                    process.BeginErrorReadLine();
                    await Task.Run(() => process.WaitForExit());
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }

        static void OutputHandler(object sendingProcess, DataReceivedEventArgs outLine, TextBox outputTextBox)
        {
            if (outLine.Data != null)
            {
                // Use Invoke to safely update the TextBox from another thread
                outputTextBox.Invoke((MethodInvoker)delegate
                {
                    outputTextBox.AppendText(outLine.Data + Environment.NewLine);
                });
            }
        }

        static void RunExecutable(string path, string arguments)
        {
            MessageBox.Show(path + " " + arguments);
            try
            {
                using (Process process = new Process())
                {
                    process.StartInfo.FileName = path;
                    process.StartInfo.Arguments = arguments;
                    process.StartInfo.UseShellExecute = false;
                    process.StartInfo.RedirectStandardOutput = true;
                    process.StartInfo.CreateNoWindow = false;

                    process.Start();
                    string output = process.StandardOutput.ReadToEnd();
                    process.WaitForExit();

                    // Display the output in the console
                    Console.WriteLine("Output: " + output);

                    // Display a success message in the console
                    Console.WriteLine("成功");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message);
            }
        }

        private async void executeButton_Click(object sender, EventArgs e)
        {
            string executablePath = @".\simple-vad.exe";
            string args = "";

            if (!checkBoxUseFiltering.Checked)
            {
                args += "--no-filtering ";
            }

            foreach (var kvp in param)
            {
                args += kvp.Key + " " + kvp.Value + " ";
            }

            args += "\"" + inputTextBox.Text + "\"";

            executeButton.Enabled = false;
            await RunExecutableAsync(executablePath, args);
            executeButton.Enabled = true;
        }

        private void outputButton_Click(object sender, EventArgs e)
        {
            // Create an instance of OpenFileDialog
            SaveFileDialog dialog = new SaveFileDialog();

            // Set properties for the OpenFileDialog
            dialog.Title = "选择保存时间轴文件的位置";
            dialog.Filter = "Subtitle files (*.srt)|*.srt";
            dialog.FilterIndex = 1; // Default to the second filter (Text Files)
            dialog.RestoreDirectory = true;

            // Show the OpenFileDialog and check if the user clicked OK
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                // Get the selected file name
                string selectedFileName = dialog.FileName;
                outputTextbox.Text = selectedFileName;

                updateParam("-o", "\"" + selectedFileName + "\"");
            }
        }

        private void minFreqPicker_ValueChanged(object sender, EventArgs e)
        {

            updateParam("--min-freq", minFreqPicker.Value.ToString());
        }

        private void maxFreqPicker_ValueChanged(object sender, EventArgs e)
        {
            updateParam("--max-freq", maxFreqPicker.Value.ToString());
        }

        private void vadModePicker_ValueChanged(object sender, EventArgs e)
        {
            updateParam("--vad-mode", vadModePicker.Value.ToString());
        }

        private void minDurationPicker_ValueChanged(object sender, EventArgs e)
        {
            updateParam("--min-valid-duration", minDurationPicker.Value.ToString());
        }

        private void mergeThresholdPicker_ValueChanged(object sender, EventArgs e)
        {
            updateParam("--merge-threshold", mergeThresholdPicker.Value.ToString());
        }

        private void minGapPicker_ValueChanged(object sender, EventArgs e)
        {
            updateParam("--min-gap-duration", minGapPicker.Value.ToString());
        }

        private void numericUpDown1_ValueChanged_1(object sender, EventArgs e)
        {
            updateParam("--min-clear-ratio", minClearRatioPicker.Value.ToString());
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            // Check if the data is a file or a list of files
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                // Allow dropping of the data
                e.Effect = DragDropEffects.Copy;
            }
            else
            {
                // Disallow dropping
                e.Effect = DragDropEffects.None;
            }
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            // Get the file or files dropped
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);

            if (files != null && files.Length > 0 && files[0].EndsWith(".wav"))
            {
                inputTextBox.Text = files[0];
                executeButton.Enabled = files[0].Length > 0;
            }
        }
    }

    public static class ProcessExtensions
    {
        public static Task<bool> WaitForExitAsync(this Process process)
        {
            var tcs = new TaskCompletionSource<bool>();
            process.EnableRaisingEvents = true;
            process.Exited += (s, e) => tcs.TrySetResult(true);
            return tcs.Task;
        }
    }
}