using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace z80_opcode_decode
{
    public partial class z80decoder : Form
    {
        public z80decoder()
        {
            InitializeComponent();
        }

        private void txtOpcode_TextChanged(object sender, EventArgs e)
        {
            if (((TextBox)sender).Focused == false)
            {
                return;
            }

            int opcode = 0;
            int x = 0;
            int y = 0;
            int z = 0;
            int p = 0;
            int q = 0;

            try
            {
                if (sender == txtOpcode)
                {
                    opcode = (int.Parse(txtOpcode.Text.Substring(2), System.Globalization.NumberStyles.HexNumber) & 0xFF);

                    x = (opcode >> 6);
                    y = (opcode >> 3) & 0x7;
                    z = (opcode & 0x7);
                    p = (opcode >> 4) & 0x3;
                    q = (opcode >> 3) & 0x1;

                    txtX.Text = "" + x;
                    txtY.Text = "" + y;
                    txtZ.Text = "" + z;
                    txtP.Text = "" + p;
                    txtQ.Text = "" + q;
                }
                else
                {
                    x = int.Parse(txtX.Text) & 0x3;
                    z = int.Parse(txtZ.Text) & 0x7;

                    if (sender == txtP || sender == txtQ) // recalc Y
                    {
                        p = int.Parse(txtP.Text) & 0x3;
                        q = int.Parse(txtQ.Text) & 0x1;
                        y = (p << 1) | q;

                        txtY.Text = "" + y;
                    }
                    else // recalc P and Q
                    {
                        y = int.Parse(txtY.Text) & 0x7;
                        p = y >> 1;
                        q = y & 0x1;

                        txtP.Text = "" + p;
                        txtQ.Text = "" + q;
                    }

                    // put opcode together
                    opcode = (x << 6) | (y << 3) | z;
                    txtOpcode.Text = "0x" + opcode.ToString("X");
                }
            }
            catch
            {
                return;
            }
        }
    }
}
