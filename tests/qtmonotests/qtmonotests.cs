
public delegate void Triggered(int a, string b, float c);

public class TestReceiver {

	public static void Main(string[] args) {
		System.Console.WriteLine("Main called.");
	}

	public static void Test(dynamic obj) {
		obj.triggered((Triggered)((a, b, c) => {
			System.Console.WriteLine("Called");
		}));
	}
}
