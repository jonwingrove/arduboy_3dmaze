using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SpriteEditor
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private List<Button> m_createdButtons = new List<Button>();
        int[,] m_cols;
        int m_width;
        int m_height;

        private Color[] m_colors =
        {
            Color.Aqua,
            Color.White,
            Color.Gray,
            Color.Black
        };

        void RefreshView()
        {
            int idx = 0;
            for (int xx = 0; xx < m_width; ++xx)
            {
                for (int yy = 0; yy < m_height; ++yy)
                {
                    m_createdButtons[idx].BackColor = m_colors[m_cols[xx, yy]];
                    ++idx;
                }
            }
        }

        void SpriteToText()
        {
            string output = "";
            for(int y = 0; y < m_height; ++y)
            {
                output += "\n";
                for(int x = 0; x < m_width; x+=4)
                {
                    uint v = 0;
                    v += (uint)m_cols[x+3, y];
                    v += (uint)(m_cols[x+2, y]) << 2;
                    v += (uint)(m_cols[x + 1, y]) << 4;
                    v += (uint)(m_cols[x + 0, y]) << 6;

                    byte r = (byte)v;
                    string val = string.Format("0x{0:x2}", r);

                    output += val;
                    output += ", ";
                }
                output += "\r";
            }
            textBox1.Text = output;
        }

        void TextToSprite()
        {
            string ts = textBox1.Text;
            string[] splits = ts.Split(",".ToCharArray());
            int x = 0;
            int y = 0;

            foreach(string sp in splits)
            {
                string sps = sp.Replace(" ", "").Replace("\n", "").Replace("\r","");
                if (sps.Length >= 2)
                {
                    uint v = Convert.ToUInt32(sps.Substring(2), 16);
                    uint d = (v) & 3;
                    uint c = (v >> 2) & 3;
                    uint b = (v >> 4) & 3;
                    uint a = (v >> 6) & 3;

                    //Console.WriteLine(v + " " + a + " " + b + " " + c + " " + d);

                    m_cols[x, y] = (int)a;
                    x++; if (x == m_width) { x = 0; y++; }
                    m_cols[x, y] = (int)b;
                    x++; if (x == m_width) { x = 0; y++; }
                    m_cols[x, y] = (int)c;
                    x++; if (x == m_width) { x = 0; y++; }
                    m_cols[x, y] = (int)d;
                    x++; if (x == m_width) { x = 0; y++; }
                    
                }
            }

            RefreshView();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            foreach(Button b in m_createdButtons)
            {
                panel1.Controls.Remove(b);
            }
            m_createdButtons.Clear();

            m_width = System.Convert.ToInt32(widthTextBox.Text);
            m_height = System.Convert.ToInt32(heightTextBox.Text);

            m_cols = new int[m_width, m_height];

            int size = (int)((float)Math.Min(panel1.Width, panel1.Height) / (float)Math.Max(m_width, m_height));

            for (int xx=0;xx< m_width; ++xx)
            {
                for(int yy=0;yy< m_height; ++yy)
                {
                    Button nb = new Button();
                    panel1.Controls.Add(nb);
                    nb.Width = size;
                    nb.Height = size;
                    nb.Left = xx * size;
                    nb.Top = yy * size;

                    int tx = xx;
                    int ty = yy;
                    nb.Click += (s, args) =>
                    {
                        m_cols[tx, ty]++;
                        if(m_cols[tx,ty] ==4)
                        {
                            m_cols[tx, ty] = 0;
                        }
                        RefreshView();
                    };

                    m_createdButtons.Add(nb);
                }
            }

            RefreshView();
        }

        private void CodeToSpriteButton_Click(object sender, EventArgs e)
        {
            TextToSprite();
        }

        private void SpriteToCodeButton_Click(object sender, EventArgs e)
        {
            SpriteToText();
        }
    }
}
