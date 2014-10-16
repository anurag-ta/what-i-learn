#!/usr/bin/perl

use Spreadsheet::WriteExcel; #Used for Writing Excel sheets 
use Spreadsheet::WriteExcel::Utility;
use Spreadsheet::ParseExcel;
use Spreadsheet::ParseExcel::SaveParser;
use File::Copy;
our $row=1,$col=0;
our $counter =1;

date_time();
our $sheetname = $ARGV[0];
$sheetname =~m/lmac_(.*).txt/;                  #sheetname is taking from category of testing
$sheetname = $1;                                #gives first pattern matching from the category name

$xl_dir = "./Test_results/";
$xl_fname = "uu_acmac_lmac-$months[$month]-$dayOfMonth-$year-TestResults.xls";  #name of the file
$backup_filename = "uu_acmac_lmac_$months[$month]-$dayOfMonth-$year-TestResults.xls";

our $parser = Spreadsheet::ParseExcel::SaveParser->new();   #creates Parser object required to open excel file
my $xl_file = $parser->Parse($xl_dir.$xl_fname);   #Opens excel file and returns SaveParser::Workbook object

if($ARGV[1] eq "")                      #If single test case is running then excel file will not be created
{
    if($xl_file eq "")                  #checks whether excel file is exists or not, If not file will be created here
    {
        $xl_file= Spreadsheet::WriteExcel->new($xl_dir.$xl_fname);      #creating new excel file
            die "problem creating new excel file: $!" unless defined $xl_file;      #if file opening error occurs it will die

            our $sheet = $xl_file->add_worksheet($sheetname);       #adding new worksheet
            our $format_heading=$xl_file->add_format(color=>'black','bold');        #format changes of font in sheet
            excel_heading($sheet);

    }

    else                                                #if file exist, adding of new sheet will be done here
    {
        copy($xl_dir.$xl_fname,$xl_dir.$backup_filename) or die "copy failed: $!"; #making backup file
            $xl_file = $parser->Parse($xl_dir.$xl_fname);    #Opens excel file and returns SaveParser::Workbook object
            $xl_file = $xl_file->SaveAs($xl_dir.$xl_fname);   #SaveAs method will return WriteExcel::Workbook object
            $duplicate_sheetname=$sheetname;

        foreach my $get_sheetname ($xl_file->sheets()) 
        {
            our $check_sheetname= $get_sheetname->get_name();
            if($check_sheetname eq $sheetname)
            {
                $sheetname=$duplicate_sheetname.$counter;
                $counter++;
            }
        }

        our $sheet = $xl_file->add_worksheet($sheetname);       #adding new worksheet
            $sheet_count =$xl_file->sheets();                 #getting the list of sheets in the excel file
            $sheet_count = $sheet_cnt-1;
        our $sheet =$xl_file->sheets($sheet_count);       #reading of newly added sheet
            our $format_heading=$xl_file->add_format(color=>'black','bold');  #format changes for font in sheet
            excel_heading($sheet);
        copy($xl_dir.$backup_filename,$xl_dir.$xl_fname) or die "copy failed: $!"; #copying the backup to orginal file
            unlink($xl_dir.$backup_filename);                       #deleting the backup file
    }
}

sub excel_heading
{
    $sheet = shift;
    $sheet->set_column('A:A',50);           #setting the column width to 50 characters
        $sheet->set_column('B:B',20);           #setting the column width to 20 characters
        $sheet->set_column('C:C',30);           #setting the column width to 30 characters
        $sheet->write('0','0','Testcase Id',$format_heading);   #Testcase Id as heading
        $sheet->write('0','1','Status',$format_heading);        #Status as heading
        $sheet->write('0','2','Date',$format_heading);          #Date as heading
}


#Gives the status and date for the testcase id
sub excelsheet
{
    my $xl_file = $parser->Parse($xl_dir.$xl_fname);
    $xl_file= $xl_file->SaveAs($xl_dir.$xl_fname);
    $sheet =$xl_file->sheets($sheet_count);
    $sheet->write($row,$col,$global_testcase_num); #writing testcase num in the first column of sheet
        if($casefailed eq "0")                      #Writing status for the testcase
        {
            $col=$col+1;
            $sheet->write($row,$col,'success');
        }

        else
        {
            $col=$col+1;
            $sheet->write($row,$col,'failed');
        }
    $col=$col+1;
    $sheet->write($row,$col,$theTime);          #writing date and time for the testcase 
        $row++, $col=0;
}
1
