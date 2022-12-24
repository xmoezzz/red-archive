namespace NekoPackExtract
{
    partial class Form1
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ProgressBar = new System.Windows.Forms.ProgressBar();
            this.PackButton = new System.Windows.Forms.Button();
            this.OutputFileNameButton = new System.Windows.Forms.Button();
            this.PackDirectoryButton = new System.Windows.Forms.Button();
            this.OriginalPackButton = new System.Windows.Forms.Button();
            this.OutputFileNameTextBox = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.PackDirectoryTextBox = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.OriginalPackTextBox = new System.Windows.Forms.TextBox();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(129, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(287, 15);
            this.label1.TabIndex = 0;
            this.label1.Text = "Drop dat files or mng files here>_<";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ProgressBar);
            this.groupBox1.Controls.Add(this.PackButton);
            this.groupBox1.Controls.Add(this.OutputFileNameButton);
            this.groupBox1.Controls.Add(this.PackDirectoryButton);
            this.groupBox1.Controls.Add(this.OriginalPackButton);
            this.groupBox1.Controls.Add(this.OutputFileNameTextBox);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.PackDirectoryTextBox);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.OriginalPackTextBox);
            this.groupBox1.Location = new System.Drawing.Point(12, 27);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(546, 175);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Repack setting";
            // 
            // ProgressBar
            // 
            this.ProgressBar.Location = new System.Drawing.Point(9, 140);
            this.ProgressBar.Name = "ProgressBar";
            this.ProgressBar.Size = new System.Drawing.Size(446, 23);
            this.ProgressBar.TabIndex = 2;
            // 
            // PackButton
            // 
            this.PackButton.Location = new System.Drawing.Point(461, 140);
            this.PackButton.Name = "PackButton";
            this.PackButton.Size = new System.Drawing.Size(75, 23);
            this.PackButton.TabIndex = 9;
            this.PackButton.Text = "Pack!";
            this.PackButton.UseVisualStyleBackColor = true;
            this.PackButton.Click += new System.EventHandler(this.PackButton_Click);
            // 
            // OutputFileNameButton
            // 
            this.OutputFileNameButton.Location = new System.Drawing.Point(461, 101);
            this.OutputFileNameButton.Name = "OutputFileNameButton";
            this.OutputFileNameButton.Size = new System.Drawing.Size(75, 23);
            this.OutputFileNameButton.TabIndex = 8;
            this.OutputFileNameButton.Text = "Select";
            this.OutputFileNameButton.UseVisualStyleBackColor = true;
            this.OutputFileNameButton.Click += new System.EventHandler(this.OutputFileNameButton_Click);
            // 
            // PackDirectoryButton
            // 
            this.PackDirectoryButton.Location = new System.Drawing.Point(461, 64);
            this.PackDirectoryButton.Name = "PackDirectoryButton";
            this.PackDirectoryButton.Size = new System.Drawing.Size(75, 23);
            this.PackDirectoryButton.TabIndex = 7;
            this.PackDirectoryButton.Text = "Select";
            this.PackDirectoryButton.UseVisualStyleBackColor = true;
            this.PackDirectoryButton.Click += new System.EventHandler(this.PackDirectoryButton_Click);
            // 
            // OriginalPackButton
            // 
            this.OriginalPackButton.Location = new System.Drawing.Point(461, 27);
            this.OriginalPackButton.Name = "OriginalPackButton";
            this.OriginalPackButton.Size = new System.Drawing.Size(75, 23);
            this.OriginalPackButton.TabIndex = 6;
            this.OriginalPackButton.Text = "Select";
            this.OriginalPackButton.UseVisualStyleBackColor = true;
            this.OriginalPackButton.Click += new System.EventHandler(this.OriginalPackButton_Click);
            // 
            // OutputFileNameTextBox
            // 
            this.OutputFileNameTextBox.Location = new System.Drawing.Point(155, 98);
            this.OutputFileNameTextBox.Name = "OutputFileNameTextBox";
            this.OutputFileNameTextBox.Size = new System.Drawing.Size(300, 25);
            this.OutputFileNameTextBox.TabIndex = 5;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 101);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(135, 15);
            this.label4.TabIndex = 4;
            this.label4.Text = "Output Filename:";
            // 
            // PackDirectoryTextBox
            // 
            this.PackDirectoryTextBox.Location = new System.Drawing.Point(155, 61);
            this.PackDirectoryTextBox.Name = "PackDirectoryTextBox";
            this.PackDirectoryTextBox.Size = new System.Drawing.Size(300, 25);
            this.PackDirectoryTextBox.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 64);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(127, 15);
            this.label3.TabIndex = 2;
            this.label3.Text = "Pack Directory:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 27);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(119, 15);
            this.label2.TabIndex = 1;
            this.label2.Text = "Original Pack:";
            // 
            // OriginalPackTextBox
            // 
            this.OriginalPackTextBox.Location = new System.Drawing.Point(156, 24);
            this.OriginalPackTextBox.Name = "OriginalPackTextBox";
            this.OriginalPackTextBox.Size = new System.Drawing.Size(299, 25);
            this.OriginalPackTextBox.TabIndex = 0;
            this.OriginalPackTextBox.TextChanged += new System.EventHandler(this.OriginalPackTextBox_TextChanged);
            // 
            // Form1
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(570, 214);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "NekoPackExtract";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.Form1_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.Form1_DragEnter);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox OriginalPackTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox PackDirectoryTextBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox OutputFileNameTextBox;
        private System.Windows.Forms.Button OriginalPackButton;
        private System.Windows.Forms.Button PackDirectoryButton;
        private System.Windows.Forms.Button OutputFileNameButton;
        private System.Windows.Forms.Button PackButton;
        private System.Windows.Forms.ProgressBar ProgressBar;
    }
}

