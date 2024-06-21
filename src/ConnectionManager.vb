Imports System.Net.Http
Imports System.Net.Http.Headers
Imports System.IO
Imports System.Text

Partial Public Class Form1

    Public Enum movementDirection
        Forward
        Backward
        Left
        Right

        ForwardAndLeft
        ForwardAndRight
        BackwardAndLeft
        BackwardAndRight

        Halt
    End Enum

    Public Enum launchCategory
        Server
        Sim
        Navigating
        Mapping
        LiDAR
        PiCam
    End Enum

    Dim client
    Public Function connectToServer(ip As String) As Boolean
        client = New HttpClient()
        Try
            client.BaseAddress = New Uri("http://" & ip)
            client.Timeout = TimeSpan.FromSeconds(45)

            Return True
        Catch
            client.dispose()
            Return False
        End Try
    End Function

    Private Async Function pollingLoop() As Task
        Try
            While (Await timer.WaitForNextTickAsync(cts.Token))
                If connected Then
                    Dim readSensorDataTask = Task.WhenAll(getSensorDataAsync(infoType.powerConsumption), getSensorDataAsync(infoType.remainingCharge), getSensorDataAsync(infoType.batteryVoltage))
                    Await readSensorDataTask
                End If
            End While
        Catch ex As OperationCanceledException
            rtbLogs.AppendText("Operation cancelled exception - No longer connected")
        End Try
    End Function

    Dim polledData_powerConsumption, polledData_remainingCharge, polledData_batteryVoltage
    Private Sub storeSensorData(data As String, sensor As infoType)
        If (sensor = infoType.powerConsumption) Then
            polledData_powerConsumption = data
        End If

        If (sensor = infoType.remainingCharge) Then
            polledData_remainingCharge = data
        End If

        If (sensor = infoType.batteryVoltage) Then
            polledData_batteryVoltage = data
        End If
    End Sub

    Public Async Function getSensorDataAsync(sensor As infoType) As Task(Of Boolean)
        Dim sensorEndpoint As String
        If (sensor = infoType.powerConsumption) Then sensorEndpoint = "/readPowerConsumption"
        If (sensor = infoType.remainingCharge) Then sensorEndpoint = "/readRemainingCharge"
        If (sensor = infoType.batteryVoltage) Then sensorEndpoint = "/readBatteryVoltage"

        Dim result As String = Await resourceRequestAsync(sensorEndpoint)

        updateInfoLabels(result, sensor)

        If result <> "ERROR" Then
            storeSensorData(result, sensor)
            Return True
        End If

        Return False
    End Function

    Dim currentMovementDirection As movementDirection = movementDirection.Halt
    Public Async Function sendMoveInstructionAsync(direction As movementDirection) As Task(Of Boolean)
        If currentMovementDirection = direction Then Return True

        currentMovementDirection = direction

        Dim directionEndpoint As String
        If (direction = movementDirection.Forward) Then directionEndpoint = "/moveForward"
        If (direction = movementDirection.Backward) Then directionEndpoint = "/moveBackward"
        If (direction = movementDirection.Left) Then directionEndpoint = "/turnLeft"
        If (direction = movementDirection.Right) Then directionEndpoint = "/turnRight"

        If (direction = movementDirection.ForwardAndLeft) Then directionEndpoint = "/moveForwardAndLeft"
        If (direction = movementDirection.ForwardAndRight) Then directionEndpoint = "/moveForwardAndRight"
        If (direction = movementDirection.BackwardAndLeft) Then directionEndpoint = "/moveBackwardAndLeft"
        If (direction = movementDirection.BackwardAndRight) Then directionEndpoint = "/moveBackwardAndRight"

        If (direction = movementDirection.Halt) Then directionEndpoint = "/haltMovement"

        Dim result As String = Await resourceRequestAsync(directionEndpoint)

        rtbLogs.AppendText(result + vbCrLf)

        If (result <> "ERROR") Then
            Return True
        End If

        Return False

    End Function

    Public Async Function sendLaunchRequest(category As launchCategory) As Task(Of Boolean)
        Dim categoryEndpoint, result As String
        If (category = launchCategory.Server) Then categoryEndpoint = "/launchServer"
        If (category = launchCategory.Sim) Then categoryEndpoint = "/launchSim"
        If (category = launchCategory.Mapping) Then categoryEndpoint = "/launchMapping"
        If (category = launchCategory.Navigating) Then
            categoryEndpoint = "/launchNavigating"

            Dim fileDialog = New OpenFileDialog()
            fileDialog.Filter = "YAML|*.yaml;*.yml"
            fileDialog.RestoreDirectory = True

            If (fileDialog.ShowDialog = DialogResult.OK) Then
                result = Await resourceProvideAsync(categoryEndpoint, fileDialog.FileName)
            Else
                Return False
            End If

        Else
            result = Await resourceRequestAsync(categoryEndpoint)
        End If

        rtbLogs.AppendText(result + vbCrLf)

        If (result <> "ERROR") Then
            Return True
        End If

        Return False
    End Function

    Public Async Function sendSensorStartRequest(category As launchCategory) As Task(Of Boolean)
        Dim categoryEndpoint As String
        If (category = launchCategory.PiCam) Then categoryEndpoint = "/startPiCam"
        If (category = launchCategory.LiDAR) Then categoryEndpoint = "/startLiDAR"

        Try
            Dim content As New StringContent(tbRobotIP.Text, Encoding.UTF8, "application/json")
            Dim response As HttpResponseMessage = Await client.PostAsync(categoryEndpoint, content)
            response.EnsureSuccessStatusCode()
            Dim responseContent As String = Await response.Content.ReadAsStringAsync()
            rtbLogs.AppendText(responseContent)

            Return True
        Catch e As Exception
            Return False
        End Try
    End Function

    Public Async Function sendToggleAutoAsync() As Task(Of Boolean)
        Dim result As String = Await resourceRequestAsync("/toggleAutonomousNavigation")

        rtbLogs.AppendText(result + vbCrLf)

        If (result <> "ERROR") Then
            Return True
        End If
        Return False

    End Function

    Public Async Function startFoxgloveBridge() As Task(Of Boolean)
        Dim result As String = Await resourceRequestAsync("/startFoxgloveBridge")

        rtbLogs.AppendText(result + vbCrLf)

        If (result <> "ERROR") Then Return True

        Return False
    End Function


    Private Async Function resourceRequestAsync(endpoint As String) As Task(Of String)
        If Not connected Then
            Return "ERROR"
        End If

        Try
            Dim responseTask As Task(Of HttpResponseMessage) = client.GetAsync(endpoint)
            Dim response = Await responseTask

            If response.IsSuccessStatusCode Then
                Dim responseContent = response.Content.ReadAsStringAsync().Result
                Return responseContent
            Else
                Return "ERROR"
            End If
        Catch e As Exception
            rtbLogs.AppendText(e.ToString() + vbCrLf)
            Return "ERROR"
        End Try
    End Function

    Private Async Function resourceProvideAsync(endpoint As String, filePath As String) As Task(Of String)
        If Not connected Then
            Return "ERROR"
        End If

        Try
            Dim fileBytes As Byte() = File.ReadAllBytes(filePath)

            Dim content As New ByteArrayContent(fileBytes)
            content.Headers.ContentType = New System.Net.Http.Headers.MediaTypeHeaderValue("application/octet-stream")

            Dim responseTask As Task(Of HttpResponseMessage) = client.PutAsync(endpoint, content)
            Dim response = Await responseTask

            If response.IsSuccessStatusCode Then
                Dim responseContent = response.Content.ReadAsStringAsync().Result
                Return responseContent
            Else
                rtbLogs.AppendText(response.Content.ReadAsStringAsync().Result + vbCrLf)
                Return "ERROR"
            End If
        Catch e As Exception
            rtbLogs.AppendText(e.ToString() + vbCrLf)
            Return "ERROR"
        End Try
    End Function

End Class


