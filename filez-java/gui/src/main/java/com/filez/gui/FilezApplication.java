import com.filez.cli.FilezApp;
import com.filez.cli.FilezApp;
import javafx.application.Application;
import javafx.concurrent.Task;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.stage.DirectoryChooser;
import javafx.stage.Stage;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.function.Consumer;

import com.filez.gui.ExportDialog;

public class FilezApplication extends Application {
    
    private DirectoryChooser directoryChooser;
    private Path selectedDirectory;
    private TextArea logArea;
    private TextField pathField;
    private ProgressBar progressBar;
    private TabPane tabPane;
    private TableView<FileInfo> fileTable;
    private TableView<DuplicateGroup> duplicateTable;
    
    private ExecutorService executorService;
    private Process cliProcess;
    
    @Override
    public void start(Stage primaryStage) throws Exception {
        directoryChooser = new DirectoryChooser();
        directoryChooser.setTitle("Select Directory to Scan");
        directoryChooser.setInitialDirectory(new File(System.getProperty("user.home")));
        
        logArea = new TextArea();
        logArea.setEditable(false);
        logArea.setStyle("-fx-font-family: 'monospaced'; -fx-font-size: 11px;");
        logArea.setWrapText(true);
        
        pathField = new TextField();
        pathField.setPromptText("Directory:");
        pathField.setEditable(false);
        pathField.setPrefColumnCount(30);
        
        progressBar = new ProgressBar();
        progressBar.setProgress(0);
        
        Button scanButton = new Button("Scan");
        scanButton.setDefaultButton(true);
        scanButton.setOnAction(e -> startScan());
        
        Button browseButton = new Button("Browse...");
        browseButton.setOnAction(e -> browseDirectory());
        
        Button duplicatesButton = new Button("Find Duplicates");
        duplicatesButton.setOnAction(e -> findDuplicates());
        
        Button exportButton = new Button("Export Results");
        exportButton.setOnAction(e -> openExportDialog());
        
        Button exportButton = new Button("Export Results");
        exportButton.setOnAction(e -> openExportDialog());
        
        HBox pathBox = new HBox(10, new TextField(pathField));
        HBox.setHgrow(pathField);
        HBox.setMargin(new Insets(5));
        HBox.getChildren().addAll(pathField, browseButton);
        
        HBox buttonBox = new HBox(10, scanButton, duplicatesButton);
        buttonBox.setSpacing(10);
        buttonBox.setPadding(new Insets(0, 0, 15));
        
        VBox vbox = new VBox(20, pathBox, buttonBox, new Separator(), progressBar, logArea);
        vbox.setSpacing(10);
        vbox.setPadding(new Insets(10));
        vbox.setFillWidth(true);
        
        fileTable = new TableView<>();
        setupFileTable();
        
        duplicateTable = new TableView<>();
        setupDuplicateTable();
        
        tabPane = new TabPane();
        Tab filesTab = new Tab("Files", fileTable);
        Tab duplicatesTab = new Tab("Duplicates", duplicateTable);
        tabPane.getTabs().addAll(filesTab, duplicatesTab);
        
        VBox resultsBox = new VBox(10, tabPane);
        resultsBox.setSpacing(10);
        resultsBox.setVgrow(tabPane);
        
        Scene scene = new Scene(topBox, 900, 700);
        primaryStage.setTitle("filez v" + FilezCore.FO_VERSION);
        primaryStage.setScene(scene);
        primaryStage.show();
    }
    
    private void setupFileTable() {
        TableColumn<FileInfo, String> pathColumn = new TableColumn<>("Path");
        pathColumn.setCellValueFactory(param -> param.pathProperty().get());
        pathColumn.setPrefWidth(300);
        
        TableColumn<FileInfo, Long> sizeColumn = new TableColumn<>("Size");
        sizeColumn.setCellValueFactory(param -> param.sizeProperty().get());
        sizeColumn.setPrefWidth(100);
        
        TableColumn<FileInfo, String> typeColumn = new TableColumn<>("Type");
        typeColumn.setCellValueFactory(param -> param.mimeType().get());
        typeColumn.setPrefWidth(100);
        
        fileTable.getColumns().addAll(pathColumn, sizeColumn, typeColumn);
    }
    
    private void setupDuplicateTable() {
        TableColumn<DuplicateGroup, String> hashColumn = new TableColumn<>("Hash");
        hashColumn.setCellValueFactory(param -> param.hashes().blake3Property().get());
        hashColumn.setPrefWidth(150);
        
        TableColumn<DuplicateGroup, Long> fileCountColumn = new TableColumn<>("Count");
        fileCountColumn.setCellValueFactory(param -> param.fileCountProperty().get());
        fileCountColumn.setPrefWidth(80);
        
        TableColumn<DuplicateGroup, Long> totalSizeColumn = new TableColumn<>("Total Size");
        totalSizeColumn.setCellValueFactory(param -> param.totalSizeProperty().get());
        totalSizeColumn.setPrefWidth(120);
        
        duplicateTable.getColumns().addAll(hashColumn, fileCountColumn, totalSizeColumn);
    }
    
    private void browseDirectory() {
        File dir = directoryChooser.showDialog(new Stage());
        if (dir != null) {
            selectedDirectory = dir.toPath();
            pathField.setText(dir.getAbsolutePath());
            appendLog("Selected directory: " + dir.getAbsolutePath());
        }
    }
    
    private void startScan() {
        Path dir = selectedDirectory;
        if (dir == null) {
            appendLog("Please select a directory first.");
            return;
        }
        
        appendLog("Starting scan: " + dir);
        progressBar.setProgress(ProgressIndicator.INDETERMINATE_PROGRESS);
        
        Task<Void> scanTask = new Task<Void>() {
            @Override
            protected Void call() {
                try {
                    List<String> args = List.of("scan", dir.toString(), "--format", "json");
                    int exitCode = runCliCommand(args, output -> {
                        appendLog(output);
                        progressBar.setProgress(ProgressIndicator.INDETERMINATE_PROGRESS);
                    });
                    
                    if (exitCode == 0) {
                        appendLog("Scan completed successfully.");
                    } else {
                        appendLog("Scan failed with exit code: " + exitCode);
                    }
                }
            }
        };
        
        progressBar.progressProperty().bind(scanTask.progressProperty());
        
        executorService = ExecutorService.newSingleThreadExecutor();
        executorService.submit(scanTask);
    }
    
    private void findDuplicates() {
        Path dir = selectedDirectory;
        if (dir == null) {
            appendLog("Please select a directory first.");
            return;
        }
        
        appendLog("Finding duplicates: " + dir);
        progressBar.setProgress(ProgressIndicator.INDETERMINATE_PROGRESS);
        
        Task<Void> dupTask = new Task<Void>() {
            @Override
            protected Void call() throws Exception {
                try {
                    List<String> args = List.of("duplicates", dir.toString(), "--format", "json");
                    int exitCode = runCliCommand(args, output -> {
                        appendLog(output);
                        progressBar.setProgress(ProgressIndicator.INDETERMINATE_PROGRESS);
                    });
                    
                    if (exitCode == 0) {
                        appendLog("Duplicate search completed successfully.");
                    } else {
                        appendLog("Duplicate search failed with exit code: " + exitCode);
                    }
                }
            }
        };
        
        progressBar.progressProperty().bind(dupTask.progressProperty());
        
        executorService = ExecutorService.newSingleThreadExecutor();
        executorService.submit(dupTask);
    }
    
    private int runCliCommand(List<String> args, Consumer<String> outputConsumer) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(getJavaCommand());
        pb.command().addAll(args);
        
        Process process = pb.start();
        StringBuilder output = new StringBuilder();
        
        try (var reader = process.inputReader()) {
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
                outputConsumer.accept(line);
            }
        } catch (Exception e) {
            // If reading fails, process might have already completed
        }
        
        return process.waitFor();
    }
    
    private String getJavaCommand() {
        String javaHome = System.getProperty("java.home");
        if (javaHome == null) {
            javaHome = System.getProperty("user.dir");
        }
        
        String classpath = System.getProperty("java.class.path");
        
        if (Platform.isWindows()) {
            return javaHome + File.separator + "bin" + File.separator + "java.exe";
        } else {
            return javaHome + File.separator + "bin" + File.separator + "java";
        }
    }
    
    private void appendLog(String message) {
        logArea.appendText(message + "\n");
    }
}
