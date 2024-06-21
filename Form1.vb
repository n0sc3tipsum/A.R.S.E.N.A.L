Imports System.Threading
Partial Public Class Form1

    Dim connected As Boolean = False

    Public Enum infoType As Integer
        powerConsumption
        remainingCharge
        batteryVoltage
    End Enum

    Public Sub updateInfoLabels(data As String, infoType As infoType)
        If infoType = infoType.powerConsumption Then lblPowerConsumption.Text = data
        If infoType = infoType.remainingCharge Then lblRemainingCharge.Text = data
        If infoType = infoType.batteryVoltage Then lblBatteryVoltage.Text = data
    End Sub

    Public Sub clearInfoLabels()
        lblPowerConsumption.Text = "-----"
        lblRemainingCharge.Text = "-----"
        lblBatteryVoltage.Text = "-----"
    End Sub

    Dim timer As PeriodicTimer
    Dim timertask As Task
    Dim cts As CancellationTokenSource
    Private Async Sub btnConnect_Click(sender As Object, e As EventArgs) Handles btnConnect.Click
        If connected = False Then
            If (connectToServer(tbIP.Text)) Then
                connected = True

                tbIP.Enabled = False
                lblConnectionStatus.Text = "Connected"
                btnConnect.Text = "Disconnect"

                timer = New PeriodicTimer(TimeSpan.FromMilliseconds(3000))
                cts = New CancellationTokenSource
                timertask = pollingLoop()

                Try
                    Await timertask
                Catch ex As OperationCanceledException
                    ' no longer connected
                End Try

            Else
            End If
        Else
            connected = False
            polledData_powerConsumption = ""
            polledData_remainingCharge = ""
            polledData_batteryVoltage = ""

            cts.Cancel()
            Try
                Await timertask
            Catch ex As OperationCanceledException
                ' operation canceled
            End Try
            cts.Dispose()

            client.dispose()

            tbIP.Enabled = True
            lblConnectionStatus.Text = "Not connected"
            btnConnect.Text = "Connect"
            clearInfoLabels()
        End If
    End Sub






    Private Async Sub btnToggleAutoMove_Click(sender As Object, e As EventArgs) Handles btnToggleAutoMove.Click
        Await sendToggleAutoAsync()
    End Sub


    Private Async Sub btnLeft_MouseDown(sender As Object, e As EventArgs) Handles btnLeft.MouseDown
        Await sendMoveInstructionAsync(movementDirection.Left)
    End Sub
    Private Async Sub btnRight_MouseDown(sender As Object, e As EventArgs) Handles btnRight.MouseDown
        Await sendMoveInstructionAsync(movementDirection.Right)
    End Sub
    Private Async Sub btnForward_MouseDown(sender As Object, e As EventArgs) Handles btnForward.MouseDown
        Await sendMoveInstructionAsync(movementDirection.Forward)
    End Sub
    Private Async Sub btnBackward_MouseDown(sender As Object, e As EventArgs) Handles btnBackward.MouseDown
        Await sendMoveInstructionAsync(movementDirection.Backward)
    End Sub

    Private Async Sub btnForwardAndLeft_MouseDown(sender As Object, e As EventArgs) Handles btnForwardAndLeft.MouseDown
        Await sendMoveInstructionAsync(movementDirection.ForwardAndLeft)
    End Sub
    Private Async Sub btnForwardAndRight_MouseDown(sender As Object, e As EventArgs) Handles btnForwardAndRight.MouseDown
        Await sendMoveInstructionAsync(movementDirection.ForwardAndRight)
    End Sub
    Private Async Sub btnBackwardAndLeft_MouseDown(sender As Object, e As EventArgs) Handles btnBackwardAndLeft.MouseDown
        Await sendMoveInstructionAsync(movementDirection.BackwardAndLeft)
    End Sub
    Private Async Sub btnBackwardAndRight_MouseDown(sender As Object, e As EventArgs) Handles btnBackwardAndRight.MouseDown
        Await sendMoveInstructionAsync(movementDirection.BackwardAndRight)
    End Sub

    Private Async Sub btnMovement_MouseUp(sender As Object, e As MouseEventArgs) Handles btnForward.MouseUp, btnLeft.MouseUp, btnRight.MouseUp, btnBackward.MouseUp
        Await sendMoveInstructionAsync(movementDirection.Halt)
    End Sub


    Private Async Sub Form1_KeyUp(sender As Object, e As KeyEventArgs) Handles MyBase.KeyUp
        If e.KeyCode() = Keys.W Or e.KeyCode() = Keys.A Or e.KeyCode() = Keys.S Or e.KeyCode() = Keys.D Then Then
            Await sendMoveInstructionAsync(movementDirection.Halt)
        End If
        If e.KeyCode() = Keys.Q Or e.KeyCode() = Keys.E Or e.KeyCode() = Keys.Z Or e.KeyCode() = Keys.C Then Then
            Await sendMoveInstructionAsync(movementDirection.Halt)
        End If
    End Sub
    Private Async Sub Form1_KeyDown(sender As Object, e As KeyEventArgs) Handles MyBase.KeyDown
        If e.KeyCode() = Keys.W Then
            Await sendMoveInstructionAsync(movementDirection.Forward)
        End If

        If e.KeyCode() = Keys.A Then
            Await sendMoveInstructionAsync(movementDirection.Left)
        End If

        If e.KeyCode() = Keys.S Then
            Await sendMoveInstructionAsync(movementDirection.Backward)
        End If

        If e.KeyCode() = Keys.D Then
            Await sendMoveInstructionAsync(movementDirection.Right)
        End If



        If e.KeyCode() = Keys.Q Then
            Await sendMoveInstructionAsync(movementDirection.ForwardAndLeft)
        End If

        If e.KeyCode() = Keys.E Then
            Await sendMoveInstructionAsync(movementDirection.ForwardAndRight)
        End If

        If e.KeyCode() = Keys.Z Then
            Await sendMoveInstructionAsync(movementDirection.BackwardAndLeft)
        End If

        If e.KeyCode() = Keys.C Then
            Await sendMoveInstructionAsync(movementDirection.BackwardAndRight)
        End If
    End Sub

    Const launchLabelClickText = "Attempting request send"
    Private Sub updateLaunchStatusLabel(lbl As Label, launchSuccessful As Boolean)
        If launchSuccessful Then
            lbl.Text = "Sent request successfully"
        Else
            lbl.Text = "Failed to send request"
        End If
    End Sub

    Private Async Sub btnStartNavigating_Click(sender As Object, e As EventArgs) Handles btnStartNavigating.Click
        lblStartNavigating.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartNavigating, Await sendLaunchRequest(launchCategory.Navigating))
    End Sub
    Private Async Sub btnStartServer_server_Click(sender As Object, e As EventArgs) Handles btnStartServer.Click
        lblStartServer.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartServer, Await sendLaunchRequest(launchCategory.Server))
    End Sub
    Private Async Sub btnStartSim_Click(sender As Object, e As EventArgs) Handles btnStartSim.Click
        lblStartSim.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartSim, Await sendLaunchRequest(launchCategory.Sim))
    End Sub
    Private Async Sub btnStartMapping_Click(sender As Object, e As EventArgs) Handles btnStartMapping.Click
        lblStartMapping.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartMapping, Await sendLaunchRequest(launchCategory.Mapping))
    End Sub
    Private Async Sub btnStartLiDAR_Click(sender As Object, e As EventArgs) Handles btnStartLiDAR.Click
        lblStartLiDAR.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartLiDAR, Await sendLaunchRequest(launchCategory.LiDAR))
    End Sub

    Private Async Sub btnStartPiCam_Click(sender As Object, e As EventArgs) Handles btnStartPiCam.Click
        lblStartPiCam.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartPiCam, Await sendSensorStartRequest(launchCategory.PiCam))
    End Sub
    Private Async Sub btnStartFoxgloveBridge_Click(sender As Object, e As EventArgs) Handles btnStartFoxgloveBridge.Click
        lblStartFoxgloveBridge.Text = launchLabelClickText
        updateLaunchStatusLabel(lblStartFoxgloveBridge, Await startFoxgloveBridge())
    End Sub

End Class
