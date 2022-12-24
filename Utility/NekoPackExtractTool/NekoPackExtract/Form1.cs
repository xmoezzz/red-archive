using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace NekoPackExtract
{
    public partial class Form1 : Form
    {

        public List<iUnpackObject> WorkerList = new List<iUnpackObject>();
        public Form1()
        {
            InitializeComponent();

            ClearWorker();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ResetTitle();
        }


        public void ClearWorker()
        {
            ResetTitle();
            WorkerList.Clear();
            ProgressBar.Value = 0;
            GC.Collect();
        }

        private void ResetTitle()
        {
            DateTime CompiledTime = System.IO.File.GetLastWriteTime(this.GetType().Assembly.Location);
            this.Text = "[X'moe]NekoPackExtract (built on :" + CompiledTime.ToString() + ")";
        }

        public void DisableAll()
        {
            OriginalPackButton.Enabled = false;
            PackDirectoryButton.Enabled = false;
            OutputFileNameButton.Enabled = false;
            OriginalPackTextBox.Enabled = false;
            PackDirectoryTextBox.Enabled = false;
            OutputFileNameTextBox.Enabled = false;
            PackButton.Enabled = false;

            this.AllowDrop = false;
        }

        public void EnableAll()
        {
            OriginalPackButton.Enabled = true;
            PackDirectoryButton.Enabled = true;
            OutputFileNameButton.Enabled = true;
            OriginalPackTextBox.Enabled = true;
            PackDirectoryTextBox.Enabled = true;
            OutputFileNameTextBox.Enabled = true;
            PackButton.Enabled = true;

            this.AllowDrop = true;
        }

        public void SetProgress(int CurPack)
        {
            this.Text = string.Format("Unpacking[{0}/{1}]", CurPack + 1,  WorkerList.Count);
            float Factor = (float)CurPack / (float)WorkerList.Count;
            ProgressBar.Value = (int)(Factor * 100.0);
        }


        public void SetProgressRepack(int CurIndex, int Count)
        {
            this.Text = string.Format("Unpacking[{0}/{1}]", CurIndex + 1, Count);
            float Factor = (float)CurIndex / (float)Count;
            ProgressBar.Value = (int)(Factor * 100.0);
        }

        static async Task<bool> WorkerSyncThread(Form1 This)
        {
            int Index = 0;
            foreach (iUnpackObject Item in This.WorkerList)
            {
                Item.Unpack(null);
                Index++;
            }

            return true;
        }

        static async void MonitorSyncThread(Form1 This)
        {
            This.DisableAll();

            bool Result = await WorkerSyncThread(This);

            This.EnableAll();
            This.ClearWorker();
        }

        static async Task<bool> WorkerRepackSyncThread(Form1 This)
        {
            return false; //not impl
        }

        static async void MonitorRepackSyncThread(Form1 This)
        {
            This.DisableAll();

            bool Result = await WorkerRepackSyncThread(This);

            This.EnableAll();
            This.ClearWorker();
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            String[] Files = e.Data.GetData(DataFormats.FileDrop, false) as String[];

            foreach(string FileName in Files)
            {
                try
                {
                    var FileExtensionName = Path.GetExtension(FileName);
                    
                    if (FileExtensionName.ToUpper() == ".MNG")
                    {
                        WorkerList.Add(new UnpackMNGPack(FileName));
                    }
                    else if (FileExtensionName.ToUpper() == ".DAT")
                    {
                        WorkerList.Add(new UnpackDataPack(FileName));
                    }
                }
                catch(Exception E)
                {
                }
            }

            if (WorkerList.Count == 0)
                return;

            MonitorSyncThread(this);
        }

        private void OriginalPackButton_Click(object sender, EventArgs e)
        {
            using (var Dialog = new OpenFileDialog())
            {
                Dialog.Multiselect = false;
                Dialog.Title = "Please select a nekopack (*.dat) file";
                Dialog.Filter = "NekoPack Dat(*.dat)|*.dat";
                Dialog.CheckFileExists = true;
                if (Dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    OriginalPackTextBox.Text = Dialog.FileName;
                }
            }
        }

        private void PackButton_Click(object sender, EventArgs e)
        {
            var Attribute = File.GetAttributes(OriginalPackTextBox.Text);
            if (Directory.Exists(OriginalPackTextBox.Text) || (int)Attribute == -1)
            {
                MessageBox.Show("Invalid original pack", "NekoPackExtract", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (!Directory.Exists(PackDirectoryTextBox.Text))
            {
                MessageBox.Show("Invalid pack directory", "NekoPackExtract", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            MonitorRepackSyncThread(this);
        }

        private void PackDirectoryButton_Click(object sender, EventArgs e)
        {
            using (var Dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                Dialog.Description = "Please select a folder to pack";
                if (Dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    if (string.IsNullOrEmpty(Dialog.SelectedPath))
                    {
                        MessageBox.Show("The selected path cannot be null", "NekoPackExtract", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }
                    PackDirectoryTextBox.Text = Dialog.SelectedPath;
                }
            }
        }

        private void OutputFileNameButton_Click(object sender, EventArgs e)
        {
            using (var Dialog = new OpenFileDialog())
            {
                Dialog.Multiselect = false;
                Dialog.Title = "Please specified a filename for output";
                Dialog.Filter = "NekoPack Dat(*.dat)|*.dat";
                Dialog.CheckFileExists = false;
                if (Dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    OutputFileNameTextBox.Text = Dialog.FileName;
                }
            }
        }

        private void OriginalPackTextBox_TextChanged(object sender, EventArgs e)
        {
            OutputFileNameTextBox.Text = OriginalPackTextBox.Text + ".out";
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                e.Effect = DragDropEffects.Link;
            }
            else
            {
                e.Effect = DragDropEffects.None;
            }
        }
    }
}
