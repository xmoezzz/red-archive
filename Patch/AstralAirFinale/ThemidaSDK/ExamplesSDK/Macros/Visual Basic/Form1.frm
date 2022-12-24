VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Macros Example"
   ClientHeight    =   7635
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5475
   LinkTopic       =   "Form1"
   ScaleHeight     =   7635
   ScaleWidth      =   5475
   StartUpPosition =   2  'CenterScreen
   Begin VB.Frame Frame3 
      Caption         =   "Remove Block Macro"
      Height          =   2415
      Left            =   600
      TabIndex        =   8
      Top             =   4920
      Width           =   4215
      Begin VB.CommandButton Command8 
         Caption         =   "Execute Remove Block #1"
         Height          =   375
         Left            =   720
         TabIndex        =   11
         Top             =   480
         Width           =   2655
      End
      Begin VB.CommandButton Command7 
         Caption         =   "Execute Remove Block #2"
         Height          =   375
         Left            =   720
         TabIndex        =   10
         Top             =   1080
         Width           =   2655
      End
      Begin VB.CommandButton Command2 
         Caption         =   "Execute Remove Block #3"
         Height          =   375
         Left            =   720
         TabIndex        =   9
         Top             =   1680
         Width           =   2655
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Clear Macro"
      Height          =   1695
      Left            =   600
      TabIndex        =   4
      Top             =   3000
      Width           =   4215
      Begin VB.CommandButton Command6 
         Caption         =   "Execute Clear Macro"
         Height          =   375
         Left            =   720
         TabIndex        =   5
         Top             =   480
         Width           =   2655
      End
      Begin VB.Label Label2 
         Caption         =   "If you run the macro again, an exception might occur."
         ForeColor       =   &H000000FF&
         Height          =   255
         Left            =   240
         TabIndex        =   7
         Top             =   1320
         Visible         =   0   'False
         Width           =   3855
      End
      Begin VB.Label Label1 
         Caption         =   "The code inside the macro has been deleted."
         ForeColor       =   &H000000FF&
         Height          =   255
         Left            =   480
         TabIndex        =   6
         Top             =   1080
         Visible         =   0   'False
         Width           =   3375
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Encode Macro"
      Height          =   2415
      Left            =   600
      TabIndex        =   0
      Top             =   360
      Width           =   4215
      Begin VB.CommandButton Command5 
         Caption         =   "Execute Encode Macro #3"
         Height          =   375
         Left            =   720
         TabIndex        =   3
         Top             =   1680
         Width           =   2655
      End
      Begin VB.CommandButton Command4 
         Caption         =   "Execute Encode Macro #2"
         Height          =   375
         Left            =   720
         TabIndex        =   2
         Top             =   1080
         Width           =   2655
      End
      Begin VB.CommandButton Command3 
         Caption         =   "Execute Encode Macro #1"
         Height          =   375
         Left            =   720
         TabIndex        =   1
         Top             =   480
         Width           =   2655
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Private Sub Command2_Click()

    ' the following code, inside the CODEREPLACE macro, will be removed
    ' and executed from a protected place outside the application
    
    Call VarPtr("CODEREPLACE_START")
    
    MsgBox "The CodeReplace Macro #3 has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"
   
    Call VarPtr("CODEREPLACE_END")
    

End Sub

Private Sub Command3_Click()

    ' the following code, inside the ENCODE macro, will go encrypted
    ' all the time and decrypted/encrypted again when it's executed
    
    Call VarPtr("ENCODE_START")
    
    MsgBox "The Encode Macro #1 has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"
    
    Call VarPtr("ENCODE_END")
 
End Sub

Private Sub Command4_Click()

    ' the following code, inside the ENCODE macro, will go encrypted
    ' all the time and decrypted/encrypted again when it's executed
    
    Call VarPtr("ENCODE_START")
    
    MsgBox "The Encode Macro #2 has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"
    
    Call VarPtr("ENCODE_END")
    
    
End Sub

Private Sub Command5_Click()

    ' the following code, inside the ENCODE macro, will go encrypted
    ' all the time and decrypted/encrypted again when it's executed
    
    Call VarPtr("ENCODE_START")
    
    MsgBox "The Encode Macro #3 has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"
   
    Call VarPtr("ENCODE_END")
  
End Sub

Private Sub Command6_Click()

    ' the following code, inside the CLEAR macro, will go encrypted
    ' all the time and decrypted/removed  when it's executed
    
    Call VarPtr("CLEAR_START")
    
    MsgBox "The Clear Macro has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"

    Call VarPtr("CLEAR_END")
    
    Label1.Visible = True
    Label2.Visible = True
         
End Sub

Private Sub Command7_Click()

    ' the following code, inside the CODEREPLACE macro, will be removed
    ' and executed from a protected place outside the application
    
    Call VarPtr("CODEREPLACE_START")
    
    MsgBox "The CodeReplace Macro #2 has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"
    
    Call VarPtr("CODEREPLACE_END")
    
End Sub

Private Sub Command8_Click()

    ' the following code, inside the CODEREPLACE macro, will be removed
    ' and executed from a protected place outside the application
    
    Call VarPtr("CODEREPLACE_START")
    
    MsgBox "The CodeReplace Macro #1 has successfully been executed", vbInformation + vbOKOnly, "Themida SDK example"
  
    Call VarPtr("CODEREPLACE_END")
  
End Sub
