using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private class signdata
        {
            public Int32 id;
            public byte[] value;
        }

        private void Getsgin_block(string file, out byte[] SourceAPKByte, out byte[] sginblock, out int begin, out int end)
        {
            sginblock = null;
            begin = 0;
            end = 0;
            FileStream f = File.Open(file, FileMode.Open);
            byte[] b = new byte[f.Length];
            f.Read(b, 0, (int)f.Length);
            f.Close();
            SourceAPKByte = b;
            long i = b.Length - 22;
            while (i > 0)
            {
                if (b[i] == 0x50 && b[i + 1] == 0x4b && b[i + 2] == 0x05 && b[i + 3] == 0x06)
                {
                    Console.Write(i);
                    break;
                }
                i--;
            }

            int start_central_directory_value_pos = b.Length - 6;
            int start_central_directory = BitConverter.ToInt32(b, start_central_directory_value_pos);

            byte[] byteArray = new byte[16];
            Buffer.BlockCopy(b, start_central_directory - 16, byteArray, 0, 16);

            String s = System.Text.Encoding.Default.GetString(byteArray);

            if (s == "APK Sig Block 42")
            {
                Console.Write("是二代签名！");
                int sgin_block_value_pos = start_central_directory - 16 - 8;
                Int64 sign_block_size = BitConverter.ToInt64(b, sgin_block_value_pos);
                int sign_block_start_pos = start_central_directory - (int)sign_block_size - 8;
                Int64 apkSigBlockSizeInHeader = BitConverter.ToInt64(b, sign_block_start_pos);
                if (sign_block_size != apkSigBlockSizeInHeader)
                {
                    Console.Write("错误");
                    return;
                }
                int datalen = sgin_block_value_pos + 8 - sign_block_start_pos;
                byte[] tmp = new byte[datalen];
                Buffer.BlockCopy(b, sign_block_start_pos, tmp, 0, datalen);
                sginblock = tmp;
                begin = sign_block_start_pos;
                end = sgin_block_value_pos + 8;
                return;
            }
            else
            {
                Console.Write("不是二代签名！");
            }
        }

        private void getIDValue(byte[] source, out signdata[] AIDValue)
        {
            AIDValue = null;
            List<signdata> IDValue = new List<signdata>();
            byte[] apkSignData = new byte[source.Length - 16];
            Buffer.BlockCopy(source, 8, apkSignData, 0, apkSignData.Length);
            while (true)
            {
                signdata tmp = new signdata();

                Int64 pairlen = BitConverter.ToInt64(apkSignData, 0);
                byte[] tmp1 = new byte[pairlen];
                Buffer.BlockCopy(apkSignData, 8, tmp1, 0, (int)pairlen);
                int id = BitConverter.ToInt32(tmp1, 0);
                byte[] data = new byte[pairlen - 4];
                Buffer.BlockCopy(tmp1, 4, data, 0, data.Count());
                tmp.id = id;
                tmp.value = data;
                IDValue.Add(tmp);

                byte[] tmp2 = new byte[apkSignData.Length - pairlen - 8];
                Buffer.BlockCopy(apkSignData, (int)pairlen + 8, tmp2, 0, tmp2.Length);
                apkSignData = tmp2;

                if (apkSignData.Length <= 0)
                {
                    AIDValue = IDValue.ToArray();
                    return;
                }
            }
        }

        private byte[] getNewAPKwithSign(byte[] signedAPK, int begin, int end, byte[] NewSignCode)
        {
            List<byte> result = new List<byte>();
            for (int i = 0; i < begin; i++)
            {
                result.Add(signedAPK[i]);
            }

            for (int i = 0; i < NewSignCode.Length; i++)
            {
                result.Add(NewSignCode[i]);
            }
            int len = result.Count + 16;
            byte[] tmp = BitConverter.GetBytes(len);

            for (int i = end; i < signedAPK.Length - 6; i++)
            {
                result.Add(signedAPK[i]);
            }

            for (int i = 0; i < tmp.Length; i++)
            {
                result.Add(tmp[i]);
            }
            result.Add(0);
            result.Add(0);
            return result.ToArray();
        }
        //        func getNewAPKwithSign(signedAPk []byte,b,e int,NewSigncode []byte) (result []byte){
        //    result = append(result, signedAPk[:b]...)
        //    result = append(result, NewSigncode...)
        //    i := GLConvert.Int32ToBytes_LittleEndian(len(result)+16)
        //    result = append(result, signedAPk[e:len(signedAPk)-6]...)
        //    result = append(result, i...)
        //    result = append(result, byte(0))
        //    result = append(result, byte(0))
        //    return
        //}

        private byte[] GetSignedBlock(List<signdata> src)
        {
            signdata[] IDValue = src.ToArray();
            List<byte> tmp1 = new List<byte>();
            bool needPadding = false;
            Int64 TotalSize = 0;
            int J = 0;
            for (int i = 0; i < IDValue.Length; i++)
            {
                if (IDValue[i].id == 1114793335)
                {
                    J = i;
                    needPadding = true;
                    continue;
                }

                byte[] a1 = BitConverter.GetBytes((Int64)(IDValue[i].value.Count() + 4));
                for (int j = 0; j < a1.Length; j++)
                {
                    tmp1.Add(a1[j]);
                }


                byte[] aa = BitConverter.GetBytes(IDValue[i].id);
                for (int j = 0; j < aa.Length; j++)
                {
                    tmp1.Add(aa[j]);
                }


                for (int j = 0; j < IDValue[i].value.Length; j++)
                {
                    tmp1.Add(IDValue[i].value[j]);
                }
                TotalSize = tmp1.Count;
            }
            if (needPadding)
            {
                signdata s = IDValue[J];

                Int64 b = TotalSize % 4096;

                if (b > 0)
                {
                    Int64 a = 4096 - b - 32 - 8;
                    if (a < 0)
                    {
                        a = a + 4096;
                    }
                    byte[] x = BitConverter.GetBytes(a);
                    List<byte> tmp = new List<byte>();
                    for (int j = 0; j < x.Length; j++)
                    {
                        tmp.Add(x[j]);
                    }
                    byte[] y = BitConverter.GetBytes(s.id);
                    for (int j = 0; j < y.Length; j++)
                    {
                        tmp.Add(y[j]);
                    }
                    for (int j = 0; j < a - 4; j++)
                    {
                        tmp.Add(s.value[j]);
                    }
                    for (int j = 0; j < tmp.Count; j++)
                    {
                        tmp1.Add(tmp[j]);
                    }
                }
            }

            Int64 z = tmp1.Count+24;
            byte[] Z1 = BitConverter.GetBytes(z);
            for (int j = 0; j < Z1.Count(); j++)
            {
                tmp1.Insert(j,Z1[j]);
            }
            for (int j = 0; j < Z1.Count(); j++)
            {
                tmp1.Add(Z1[j]);
            }

            return tmp1.ToArray();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {

            if (Convert.ToInt32(textBox1.Text)>100)
            {
                MessageBox.Show("请输入100以内的数字！");
                return;
            }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message.ToString()+ "\r\n请输入100以内的数字!");
                return;
            }
            if (openFileDialog1.ShowDialog() != DialogResult.OK) { return; }
            string filename = openFileDialog1.FileName; // @"F:\Pack（加入打包渠道）\app-release.apk";//
            //MessageBox.Show(filename);
            byte[] signBlock = null;
            byte[] SourceAPK;
            int begin = 0;
            int end = 0;
            Getsgin_block(filename, out SourceAPK, out signBlock, out begin, out end);
            if (signBlock==null)
            {
                MessageBox.Show("不是二代签名！");
                return;
            }
            signdata[] IDValue;
            getIDValue(signBlock, out IDValue);
            List<signdata> tmp = new List<signdata>(IDValue);
            signdata a = new signdata();
            a.id = 0x12345678;
            a.value = System.Text.Encoding.UTF8.GetBytes(this.textBox1.Text);
            bool exist = false;
            for (int i = 0; i < tmp.Count; i++)
            {
                if (tmp[i].id == a.id)
                {
                    exist = true;
                    tmp[i].value = a.value;
                }
            }
            if (!exist)
            {
                tmp.Add(a);
            }
            

            byte[] NewAPK = getNewAPKwithSign(SourceAPK, begin, end, GetSignedBlock(tmp));

            

            FileStream f = File.Open(filename.ToLower().Replace(".apk","_"+textBox1.Text+".apk"), FileMode.OpenOrCreate);
            f.Write(NewAPK, 0, NewAPK.Length);
            f.Close();
        }


        private void Form1_Load(object sender, EventArgs e)
        {
            //button1_Click(sender, e);
        }
    }
}
