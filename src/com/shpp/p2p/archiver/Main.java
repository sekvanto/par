/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package com.shpp.p2p.archiver;

public class Main implements Constants {

    public static double efficiency;  /* File compression/decompression ratio (in percents, less is better) */
    public static double time;        /* How much time operation took (in seconds) */
    public static long   fileInSize;  /* Size of input file (in bytes) */
    public static long   fileOutSize; /* Size of output file (in bytes) */

    private static String fileIn; /* Input file */
    private static String fileOut; /* Output file */
    private static boolean isArchiving; /* Set to true when archiving enabled */

    public static void main(String[] args) {
        determineInOutFile(args);
        int success;
        if (isArchiving)
            success = Archiver.archive(fileIn, fileOut);
        else
            success = Archiver.unarchive(fileIn, fileOut);
        if (success != 0) {
            System.out.println("Unsuccessful operation.");
            System.exit(success);
        }
        outputInfo();
    }

    /**
     * Takes arguments in args[], based on which determines the
     * names of input/output file
     */
    private static void determineInOutFile(String[] args) {
        /* Offset == 1 if options specified*/
        int offset = 0;
        if (args.length > 0 && (args[0].equals(ARCHIVE) || args[0].equals(UNARCHIVE)))
            offset = 1;

        /* No in/out filenames specified */
        if (args.length - offset == 0) {
            fileIn = DEFAULT_FILEIN;
            String[] newArgs = new String[args.length + 1]; /* Adding fileIn to the end of args[] */
            System.arraycopy(args, 0, newArgs, 0, offset);
            newArgs[args.length] = fileIn;
            fileOut = determineOutFile(newArgs, offset);
        }

        /* Only input filename specified */
        else if (args.length - offset == 1) {
            fileIn = args[offset];
            fileOut = determineOutFile(args, offset);
        }

        /* Both filenames specified */
        else {
            fileIn = args[offset];
            fileOut = args[offset + 1];
            isArchiving = args[0].equals(ARCHIVE) || (!args[offset].endsWith(".par") && offset == 0);
        }
    }

    /**
     * Takes arguments where ONLY input filename is specified.
     * Offset == 1 if options specified
     * Determines output filename and returns it
     */
    private static String determineOutFile(String[] args, int offset) {
        /* Archiving in these cases */
        if (args[0].equals(ARCHIVE) || (!args[0].endsWith(".par") && offset == 0)) {
            isArchiving = true;
            return args[offset] + ".par";
        }

        /* Unarchiving otherwise */
        isArchiving = false;

        if (args[offset].endsWith(".par"))
            return args[offset].substring(0, args[offset].lastIndexOf('.')) + ".uar";

        else
            return args[offset] + ".uar";
    }

    /**
     * Outputs info about operation, such as efficiency, time of operation etc
     */
    private static void outputInfo() {
        System.out.println("Input file size:   " + formatFileSize(fileInSize));
        System.out.println("Output file size:  " + formatFileSize(fileOutSize) + "\n");
        System.out.println("Compression ratio: " + efficiency + "% (less percents - higher compression, 100% - no compression)\n");
        System.out.println("Operation took: " + time + " seconds.");
    }

    /**
     * Given file size in **bytes**, determines optimal format for it, generates
     * a formatted string and returns it (e.g. "51 KB" is more optimal than "0.05 МB")
     */
    private static String formatFileSize(long fileSize) {
        double floatFileSize = (double) fileSize;
        String result = "";

        for (int i = 0; i < FSIZE_UNITS.length; i++) {
            if (floatFileSize < 1024 || i + 1 == FSIZE_UNITS.length) {
                result = i == 0 ?
                        String.format("%d %s", fileSize, FSIZE_UNITS[i]) : /* Bytes number will always be an integer */
                        String.format("%.2f %s", floatFileSize, FSIZE_UNITS[i]);
                break;
            }
            floatFileSize /= 1024;
        }

        return result;
    }

}