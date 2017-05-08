namespace SpriteEditor
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.panel1 = new System.Windows.Forms.Panel();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.widthTextBox = new System.Windows.Forms.TextBox();
            this.heightTextBox = new System.Windows.Forms.TextBox();
            this.SpriteToCodeButton = new System.Windows.Forms.Button();
            this.CodeToSpriteButton = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Location = new System.Drawing.Point(12, 12);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(448, 419);
            this.panel1.TabIndex = 0;
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(466, 76);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(178, 161);
            this.textBox1.TabIndex = 1;
            // 
            // widthTextBox
            // 
            this.widthTextBox.Location = new System.Drawing.Point(466, 12);
            this.widthTextBox.Name = "widthTextBox";
            this.widthTextBox.Size = new System.Drawing.Size(69, 20);
            this.widthTextBox.TabIndex = 2;
            this.widthTextBox.Text = "16";
            // 
            // heightTextBox
            // 
            this.heightTextBox.Location = new System.Drawing.Point(541, 12);
            this.heightTextBox.Name = "heightTextBox";
            this.heightTextBox.Size = new System.Drawing.Size(69, 20);
            this.heightTextBox.TabIndex = 3;
            this.heightTextBox.Text = "16";
            // 
            // SpriteToCodeButton
            // 
            this.SpriteToCodeButton.Location = new System.Drawing.Point(466, 243);
            this.SpriteToCodeButton.Name = "SpriteToCodeButton";
            this.SpriteToCodeButton.Size = new System.Drawing.Size(178, 23);
            this.SpriteToCodeButton.TabIndex = 4;
            this.SpriteToCodeButton.Text = "Sprite -> Code";
            this.SpriteToCodeButton.UseVisualStyleBackColor = true;
            this.SpriteToCodeButton.Click += new System.EventHandler(this.SpriteToCodeButton_Click);
            // 
            // CodeToSpriteButton
            // 
            this.CodeToSpriteButton.Location = new System.Drawing.Point(466, 272);
            this.CodeToSpriteButton.Name = "CodeToSpriteButton";
            this.CodeToSpriteButton.Size = new System.Drawing.Size(178, 23);
            this.CodeToSpriteButton.TabIndex = 5;
            this.CodeToSpriteButton.Text = "Code -> Sprite";
            this.CodeToSpriteButton.UseVisualStyleBackColor = true;
            this.CodeToSpriteButton.Click += new System.EventHandler(this.CodeToSpriteButton_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(466, 38);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(178, 23);
            this.button1.TabIndex = 6;
            this.button1.Text = "Resize";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(656, 443);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.CodeToSpriteButton);
            this.Controls.Add(this.SpriteToCodeButton);
            this.Controls.Add(this.heightTextBox);
            this.Controls.Add(this.widthTextBox);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.panel1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox widthTextBox;
        private System.Windows.Forms.TextBox heightTextBox;
        private System.Windows.Forms.Button SpriteToCodeButton;
        private System.Windows.Forms.Button CodeToSpriteButton;
        private System.Windows.Forms.Button button1;
    }
}

