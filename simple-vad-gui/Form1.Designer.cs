namespace simple_vad_gui
{

    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;



        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            checkBoxUseFiltering = new CheckBox();
            inputButton = new Button();
            inputTextBox = new TextBox();
            executeButton = new Button();
            minFreqPicker = new NumericUpDown();
            minFreqLabel = new Label();
            maxFreqLabel = new Label();
            maxFreqPicker = new NumericUpDown();
            label1 = new Label();
            energyThreshPicker = new NumericUpDown();
            outFilteredTextBox = new TextBox();
            outFilteredBtn = new Button();
            label2 = new Label();
            vadModePicker = new NumericUpDown();
            label3 = new Label();
            mergeThresholdPicker = new NumericUpDown();
            label4 = new Label();
            minDurationPicker = new NumericUpDown();
            label5 = new Label();
            minGapPicker = new NumericUpDown();
            outputTextbox = new TextBox();
            outputButton = new Button();
            stdoutBox = new TextBox();
            label6 = new Label();
            minClearRatioPicker = new NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)minFreqPicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)maxFreqPicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)energyThreshPicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)vadModePicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)mergeThresholdPicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)minDurationPicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)minGapPicker).BeginInit();
            ((System.ComponentModel.ISupportInitialize)minClearRatioPicker).BeginInit();
            SuspendLayout();
            // 
            // checkBoxUseFiltering
            // 
            checkBoxUseFiltering.AutoSize = true;
            checkBoxUseFiltering.Checked = true;
            checkBoxUseFiltering.CheckState = CheckState.Checked;
            checkBoxUseFiltering.Location = new Point(57, 77);
            checkBoxUseFiltering.Name = "checkBoxUseFiltering";
            checkBoxUseFiltering.Size = new Size(111, 24);
            checkBoxUseFiltering.TabIndex = 1;
            checkBoxUseFiltering.Text = "使用过滤器";
            checkBoxUseFiltering.UseVisualStyleBackColor = true;
            checkBoxUseFiltering.CheckedChanged += checkBoxUseFiltering_CheckedChanged;
            // 
            // inputButton
            // 
            inputButton.Location = new Point(57, 39);
            inputButton.Name = "inputButton";
            inputButton.Size = new Size(135, 29);
            inputButton.TabIndex = 2;
            inputButton.Text = "选择音频文件";
            inputButton.UseVisualStyleBackColor = true;
            inputButton.Click += inputButton_Click;
            // 
            // inputTextBox
            // 
            inputTextBox.Location = new Point(198, 40);
            inputTextBox.Name = "inputTextBox";
            inputTextBox.ReadOnly = true;
            inputTextBox.Size = new Size(539, 27);
            inputTextBox.TabIndex = 3;
            // 
            // executeButton
            // 
            executeButton.Enabled = false;
            executeButton.Location = new Point(613, 387);
            executeButton.Name = "executeButton";
            executeButton.Size = new Size(94, 29);
            executeButton.TabIndex = 4;
            executeButton.Text = "开始打轴";
            executeButton.UseVisualStyleBackColor = true;
            executeButton.Click += executeButton_Click;
            // 
            // minFreqPicker
            // 
            minFreqPicker.Increment = new decimal(new int[] { 10, 0, 0, 0 });
            minFreqPicker.Location = new Point(298, 76);
            minFreqPicker.Maximum = new decimal(new int[] { 8000, 0, 0, 0 });
            minFreqPicker.Name = "minFreqPicker";
            minFreqPicker.Size = new Size(107, 27);
            minFreqPicker.TabIndex = 5;
            minFreqPicker.ValueChanged += minFreqPicker_ValueChanged;
            // 
            // minFreqLabel
            // 
            minFreqLabel.AutoSize = true;
            minFreqLabel.Location = new Point(219, 78);
            minFreqLabel.Name = "minFreqLabel";
            minFreqLabel.Size = new Size(73, 20);
            minFreqLabel.TabIndex = 6;
            minFreqLabel.Text = "最小频率";
            // 
            // maxFreqLabel
            // 
            maxFreqLabel.AutoSize = true;
            maxFreqLabel.Location = new Point(481, 77);
            maxFreqLabel.Name = "maxFreqLabel";
            maxFreqLabel.Size = new Size(73, 20);
            maxFreqLabel.TabIndex = 8;
            maxFreqLabel.Text = "最大频率";
            // 
            // maxFreqPicker
            // 
            maxFreqPicker.Increment = new decimal(new int[] { 10, 0, 0, 0 });
            maxFreqPicker.Location = new Point(560, 76);
            maxFreqPicker.Maximum = new decimal(new int[] { 8000, 0, 0, 0 });
            maxFreqPicker.Name = "maxFreqPicker";
            maxFreqPicker.Size = new Size(107, 27);
            maxFreqPicker.TabIndex = 7;
            maxFreqPicker.Value = new decimal(new int[] { 8000, 0, 0, 0 });
            maxFreqPicker.ValueChanged += maxFreqPicker_ValueChanged;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(219, 126);
            label1.Name = "label1";
            label1.Size = new Size(73, 20);
            label1.TabIndex = 10;
            label1.Text = "最小能量";
            label1.Click += label1_Click;
            // 
            // energyThreshPicker
            // 
            energyThreshPicker.DecimalPlaces = 4;
            energyThreshPicker.Increment = new decimal(new int[] { 1, 0, 0, 196608 });
            energyThreshPicker.Location = new Point(298, 124);
            energyThreshPicker.Maximum = new decimal(new int[] { 10, 0, 0, 65536 });
            energyThreshPicker.Name = "energyThreshPicker";
            energyThreshPicker.Size = new Size(107, 27);
            energyThreshPicker.TabIndex = 9;
            energyThreshPicker.ValueChanged += numericUpDown1_ValueChanged;
            // 
            // outFilteredTextBox
            // 
            outFilteredTextBox.Location = new Point(231, 169);
            outFilteredTextBox.Name = "outFilteredTextBox";
            outFilteredTextBox.ReadOnly = true;
            outFilteredTextBox.Size = new Size(506, 27);
            outFilteredTextBox.TabIndex = 12;
            // 
            // outFilteredBtn
            // 
            outFilteredBtn.Location = new Point(57, 168);
            outFilteredBtn.Name = "outFilteredBtn";
            outFilteredBtn.Size = new Size(168, 29);
            outFilteredBtn.TabIndex = 11;
            outFilteredBtn.Text = "输出过滤后的音频";
            outFilteredBtn.UseVisualStyleBackColor = true;
            outFilteredBtn.Click += outFilteredBtn_Click;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(48, 230);
            label2.Name = "label2";
            label2.Size = new Size(73, 20);
            label2.TabIndex = 14;
            label2.Text = "识别模式";
            // 
            // vadModePicker
            // 
            vadModePicker.Location = new Point(127, 228);
            vadModePicker.Maximum = new decimal(new int[] { 3, 0, 0, 0 });
            vadModePicker.Name = "vadModePicker";
            vadModePicker.Size = new Size(107, 27);
            vadModePicker.TabIndex = 13;
            vadModePicker.Value = new decimal(new int[] { 1, 0, 0, 0 });
            vadModePicker.ValueChanged += vadModePicker_ValueChanged;
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Location = new Point(48, 286);
            label3.Name = "label3";
            label3.Size = new Size(73, 20);
            label3.TabIndex = 16;
            label3.Text = "合并阈值";
            // 
            // mergeThresholdPicker
            // 
            mergeThresholdPicker.Location = new Point(127, 286);
            mergeThresholdPicker.Maximum = new decimal(new int[] { 100000, 0, 0, 0 });
            mergeThresholdPicker.Name = "mergeThresholdPicker";
            mergeThresholdPicker.Size = new Size(107, 27);
            mergeThresholdPicker.TabIndex = 15;
            mergeThresholdPicker.Value = new decimal(new int[] { 500, 0, 0, 0 });
            mergeThresholdPicker.ValueChanged += mergeThresholdPicker_ValueChanged;
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Location = new Point(263, 230);
            label4.Name = "label4";
            label4.Size = new Size(105, 20);
            label4.TabIndex = 18;
            label4.Text = "最短有效长度";
            // 
            // minDurationPicker
            // 
            minDurationPicker.Location = new Point(374, 228);
            minDurationPicker.Maximum = new decimal(new int[] { 100000, 0, 0, 0 });
            minDurationPicker.Name = "minDurationPicker";
            minDurationPicker.Size = new Size(107, 27);
            minDurationPicker.TabIndex = 17;
            minDurationPicker.Value = new decimal(new int[] { 1000, 0, 0, 0 });
            minDurationPicker.ValueChanged += minDurationPicker_ValueChanged;
            // 
            // label5
            // 
            label5.AutoSize = true;
            label5.Location = new Point(521, 230);
            label5.Name = "label5";
            label5.Size = new Size(73, 20);
            label5.TabIndex = 20;
            label5.Text = "最断间隔";
            // 
            // minGapPicker
            // 
            minGapPicker.Location = new Point(600, 228);
            minGapPicker.Maximum = new decimal(new int[] { 100000, 0, 0, 0 });
            minGapPicker.Name = "minGapPicker";
            minGapPicker.Size = new Size(107, 27);
            minGapPicker.TabIndex = 19;
            minGapPicker.Value = new decimal(new int[] { 200, 0, 0, 0 });
            minGapPicker.ValueChanged += minGapPicker_ValueChanged;
            // 
            // outputTextbox
            // 
            outputTextbox.Location = new Point(198, 339);
            outputTextbox.Name = "outputTextbox";
            outputTextbox.ReadOnly = true;
            outputTextbox.Size = new Size(539, 27);
            outputTextbox.TabIndex = 22;
            outputTextbox.Text = "output.srt";
            // 
            // outputButton
            // 
            outputButton.Location = new Point(57, 338);
            outputButton.Name = "outputButton";
            outputButton.Size = new Size(135, 29);
            outputButton.TabIndex = 21;
            outputButton.Text = "选择输出文件名";
            outputButton.UseVisualStyleBackColor = true;
            outputButton.Click += outputButton_Click;
            // 
            // stdoutBox
            // 
            stdoutBox.BackColor = SystemColors.Desktop;
            stdoutBox.ForeColor = SystemColors.Info;
            stdoutBox.Location = new Point(57, 440);
            stdoutBox.Multiline = true;
            stdoutBox.Name = "stdoutBox";
            stdoutBox.ReadOnly = true;
            stdoutBox.ScrollBars = ScrollBars.Vertical;
            stdoutBox.Size = new Size(689, 188);
            stdoutBox.TabIndex = 23;
            // 
            // label6
            // 
            label6.AutoSize = true;
            label6.Location = new Point(264, 284);
            label6.Name = "label6";
            label6.Size = new Size(89, 20);
            label6.TabIndex = 25;
            label6.Text = "最小识别比";
            // 
            // minClearRatioPicker
            // 
            minClearRatioPicker.DecimalPlaces = 3;
            minClearRatioPicker.Increment = new decimal(new int[] { 1, 0, 0, 196608 });
            minClearRatioPicker.Location = new Point(374, 282);
            minClearRatioPicker.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
            minClearRatioPicker.Name = "minClearRatioPicker";
            minClearRatioPicker.Size = new Size(107, 27);
            minClearRatioPicker.TabIndex = 24;
            minClearRatioPicker.Value = new decimal(new int[] { 85, 0, 0, 131072 });
            minClearRatioPicker.ValueChanged += numericUpDown1_ValueChanged_1;
            // 
            // Form1
            // 
            AllowDrop = true;
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 655);
            Controls.Add(label6);
            Controls.Add(minClearRatioPicker);
            Controls.Add(stdoutBox);
            Controls.Add(outputTextbox);
            Controls.Add(outputButton);
            Controls.Add(label5);
            Controls.Add(minGapPicker);
            Controls.Add(label4);
            Controls.Add(minDurationPicker);
            Controls.Add(label3);
            Controls.Add(mergeThresholdPicker);
            Controls.Add(label2);
            Controls.Add(vadModePicker);
            Controls.Add(outFilteredTextBox);
            Controls.Add(outFilteredBtn);
            Controls.Add(label1);
            Controls.Add(energyThreshPicker);
            Controls.Add(maxFreqLabel);
            Controls.Add(maxFreqPicker);
            Controls.Add(minFreqLabel);
            Controls.Add(minFreqPicker);
            Controls.Add(executeButton);
            Controls.Add(inputTextBox);
            Controls.Add(inputButton);
            Controls.Add(checkBoxUseFiltering);
            Name = "Form1";
            Text = "SimpleVAD GUI";
            DragDrop += Form1_DragDrop;
            DragEnter += Form1_DragEnter;
            ((System.ComponentModel.ISupportInitialize)minFreqPicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)maxFreqPicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)energyThreshPicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)vadModePicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)mergeThresholdPicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)minDurationPicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)minGapPicker).EndInit();
            ((System.ComponentModel.ISupportInitialize)minClearRatioPicker).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private CheckBox checkBoxUseFiltering;
        private Button inputButton;
        private TextBox inputTextBox;
        private Button executeButton;
        private NumericUpDown minFreqPicker;
        private Label minFreqLabel;
        private Label maxFreqLabel;
        private NumericUpDown maxFreqPicker;
        private Label label1;
        private NumericUpDown energyThreshPicker;
        private TextBox outFilteredTextBox;
        private Button outFilteredBtn;
        private Label label2;
        private NumericUpDown vadModePicker;
        private Label label3;
        private NumericUpDown mergeThresholdPicker;
        private Label label4;
        private NumericUpDown minDurationPicker;
        private Label label5;
        private NumericUpDown minGapPicker;
        private TextBox outputTextbox;
        private Button outputButton;
        private TextBox stdoutBox;
        private Label label6;
        private NumericUpDown minClearRatioPicker;
    }
}