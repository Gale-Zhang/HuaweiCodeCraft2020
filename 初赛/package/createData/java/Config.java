import java.util.Random;
public interface Config {
    long RANDOM_SEED = System.currentTimeMillis();
    static Random random = new Random(RANDOM_SEED);
    String DATA_PATH = "test_data.txt";
    int V_COUNT = 30000;// + random.nextInt(7000);
    int E_COUNT = 280000;// + random.nextInt(7000);
    int MAX_MONEY = 10000;
}
