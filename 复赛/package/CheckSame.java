import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
public class CheckSame {
	public static void main(String[] args) {
		String file1 = args[0], file2 = args[1];
        BufferedReader reader1 = null, reader2 = null;
        try {
            reader1 = new BufferedReader(new FileReader(file1));
            reader2 = new BufferedReader(new FileReader(file2));
        } catch (FileNotFoundException exception) {
            System.err.println(" File Not Found");
        }
        try {
        	int idx = 1;
            String line1 = reader1.readLine(), line2 = reader2.readLine();
            while ((line1 = reader1.readLine()) != null) {
            	line2 = reader2.readLine();
                idx++;
            	if(!line1.equals(line2)) {
                    // String t1 = line1, t2 = line2;
                    // line1 = reader1.readLine();
                    // line2 = reader2.readLine();
                    // idx++;
                    // if(!t1.equals(line2)) {
                		System.out.println(idx + ": " + line1 + " " + line2);
                        break;
                    //}
            	}
            }
            reader1.close();
            reader2.close();
            System.out.println("test finished");
        } catch (IOException exception) {
            System.err.println(exception.getMessage());
        }		
	}
}
