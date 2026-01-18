package com.filez.gui;

import javafx.scene.control.*;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.control.cell.TextFieldTableCell;
import javafx.scene.control.cell.CheckBoxTableCell;
import javafx.beans.property.*;
import javafx.collections.*;

/**
 * Export dialog for filez scan results.
 * Provides options to export files to HTML, JSON, or CSV format.
 */
public class ExportDialog extends Dialog<ButtonType> {
    
    private TableView<FileInfo> fileTable;
    private ChoiceBox<String> formatChoice;
    private TextField fileNameField;
    private Button exportButton;
    private ProgressBar progressBar;
    private Label statusLabel;
    
    public ExportDialog(TableView<FileInfo> fileTable) {
        this.fileTable = fileTable;
        setTitle("Export Results");
        setHeaderText("Export Options");
        initModality(Modality.APPLICATION_MODAL);
        setResizable(true);
        setWidth(600);
    }
    
    @Override
    protected void createButtons() {
        formatChoice = new ChoiceBox<>();
        formatChoice.getItems().addAll("JSON", "CSV", "HTML");
        formatChoice.getSelectionModel().selectFirst();
        
        fileNameField = new TextField();
        fileNameField.setPromptText("Export file name:");
        fileNameField.setText("filez-export");
        
        exportButton = new Button("Export");
        exportButton.setDefaultButton(true);
        exportButton.setOnAction(e -> handleExport());
        
        progressBar = new ProgressBar();
        progressBar.setProgress(0);
        progressBar.setVisible(false);
        progressBar.setPrefWidth(400);
        
        statusLabel = new Label("Ready to export");
        statusLabel.setStyle("-fx-text-fill: #666666;");
        
        HBox formatBox = new HBox(10, new Label("Format:"), formatChoice);
        HBox nameBox = new HBox(10, new Label("File name:"), fileNameField);
        HBox buttons = new HBox(10, exportButton, ButtonType.CANCEL);
        
        VBox vbox = new VBox(15, formatBox, nameBox, buttons, new Separator(), progressBar, statusLabel);
        vbox.setSpacing(10);
        vbox.setPadding(new Insets(15));
        
        getDialogPane().getChildren().addAll(vbox);
    }
    
    private void handleExport() {
        String format = formatChoice.getValue();
        String fileName = fileNameField.getText();
        
        if (fileName == null || fileName.trim().isEmpty()) {
            fileName = "filez-export";
        }
        
        exportButton.setDisable(true);
        progressBar.setVisible(true);
        progressBar.setProgress(ProgressIndicator.INDETERMINATE_PROGRESS);
        statusLabel.setText("Exporting " + format.toUpperCase() + "...");
        
        Task<Void> exportTask = new Task<Void>() {
            @Override
            protected Void call() {
                try {
                    java.nio.file.Files.writeString(
                        java.nio.file.Path.of(fileName),
                        exportData(format)
                    );
                    
                    Thread.sleep(500);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                return null;
            }
            
            @Override
            protected void succeeded() {
                progressBar.setProgress(1);
                statusLabel.setText("Export complete!");
                exportButton.setDisable(false);
                progressBar.setVisible(false);
            }
            
            @Override
            protected void failed() {
                statusLabel.setText("Export failed: " + getException().getMessage());
                exportButton.setDisable(false);
                progressBar.setVisible(false);
            }
        };
        
        progressBar.progressProperty().bind(exportTask.progressProperty());
        Thread thread = new Thread(exportTask);
        thread.setDaemon(true);
        thread.start();
    }
    
    private String exportData(String format) {
        ObservableList<FileInfo> items = fileTable.getItems();
        StringBuilder sb = new StringBuilder();
        
        switch (format.toLowerCase()) {
            case "json":
                sb.append("[\n");
                for (FileInfo file : items) {
                    sb.append("  {\n");
                    sb.append("    \"path\": \"").append(file.path().toString().replace("\\", "\\\\")).append("\",\n");
                    sb.append("    \"size\": ").append(file.size()).append(",\n");
                    sb.append("    \"type\": \"").append(file.mimeType()).append("\",\n");
                    sb.append("  }\n");
                }
                sb.append("]\n");
                break;
                
            case "csv":
                sb.append("path,size,type\n");
                for (FileInfo file : items) {
                    sb.append(file.path().toString().replace(",", ";")).append(",");
                    sb.append(file.size()).append(",");
                    sb.append(file.mimeType()).append("\n");
                }
                break;
                
            case "html":
                sb.append("<!DOCTYPE html>\n<html>\n<head>\n");
                sb.append("<title>filez Export</title>\n");
                sb.append("<style>\n");
                sb.append("body { font-family: Arial, sans-serif; }\n");
                sb.append("<h1>Filez Scan Results</h1>\n");
                sb.append("<table>\n");
                sb.append("<tr><th>Path</th><th>Size</th><th>Type</th></tr>\n");
                for (FileInfo file : items) {
                    sb.append("<tr>\n");
                    sb.append("<td>").append(file.path().toString()).append("</td>\n");
                    sb.append("<td>").append(file.size()).append("</td>\n");
                    sb.append("<td>").append(file.mimeType()).append("</td>\n");
                    sb.append("</tr>\n");
                }
                sb.append("</table>\n");
                sb.append("</body>\n</html>\n");
                break;
                
            default:
                sb.append("Unknown format: ").append(format);
                return sb.toString();
        }
    }
}
